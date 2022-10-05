// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/ViewModelProperty.h"
#include "Mvvm/BaseViewModel.h"
#include "Mvvm/Impl/ViewModelRegistry.h"
#include "Mvvm/Impl/BindEntry.h"
#include "Mvvm/Impl/BindImpl.h"
#include "Mvvm/Impl/ViewModelPropertyIterator.h"
#include "Templates/UnrealTypeTraits.h"
#include "Templates/Function.h"
#include "Templates/EnableIf.h"

namespace UnrealMvvm_Impl
{
    struct FBlueprintPropertyChangeHandler : public IPropertyChangeHandler
    {
        FBlueprintPropertyChangeHandler(UObject* InBaseView, UFunction* InFunction)
            : BaseView(InBaseView), Function(InFunction)
        {
        }

        void Invoke(UBaseViewModel*, const FViewModelPropertyBase*) override
        {
            BaseView->ProcessEvent(Function, nullptr);
        }

        UObject* BaseView;
        UFunction* Function;
    };

    struct FNoopPropertyChangeHandler : public IPropertyChangeHandler
    {
        void Invoke(UBaseViewModel*, const FViewModelPropertyBase*) override
        {
        }
    };
}

template<typename TOwner, typename TViewModel>
class TBaseView
{
public:
    using ViewModelType = TViewModel;

    TBaseView()
    {
        // we need to have a place where Registered is used
        // simplest way - is to save a reference to static value into some variable
        // this way compiler will not optimize away initialization of Registered and this View will be recorded in Registry
        // this line though will be optimized away later and won't cost us anything
        const uint8& Register = Registered;
    }

    virtual ~TBaseView() = default;

    TViewModel* GetViewModel() const
    {
        return (TViewModel*)GetPointerToOwnerObject()->ViewModel;
    }

    void SetViewModel(TViewModel* InViewModel)
    {
        TOwner* BaseView = GetPointerToOwnerObject();
        TViewModel* OldViewModel = (TViewModel*)BaseView->ViewModel;

        if (OldViewModel && BaseView->IsConstructed())
        {
            StopListening(OldViewModel);
        }

        BaseView->ViewModel = InViewModel;
        OnViewModelChanged(OldViewModel, InViewModel);

        if (InViewModel)
        {
            if (BindEntries.Num() == 0)
            {
                PrepareBindings(BaseView);
            }

            if (BaseView->IsConstructed())
            {
                StartListening(BaseView, InViewModel);
            }

            if (!BaseView->ConstructedChanged.IsBound())
            {
                // it is safe to bind lambda here, because lifetime of this object is same as BaseView
                BaseView->ConstructedChanged.BindLambda([this](bool bConstructed) { OnViewConstructedChanged(bConstructed); });
            }
        }
    }

protected:
    /* Override this method to setup bindings. Yous should not call anything except Bind from it */
    virtual void BindProperties() {};

    /* Override this method to get notified about ViewModel changes. OldViewModel and NewViewModel may be nullptr */
    virtual void OnViewModelChanged(TViewModel* OldViewModel, TViewModel* NewViewModel) {}

private:
    template<typename T, typename P, typename C>
    friend void __BindImpl(T*, P*, C&&);

    // TBaseView may not be copied, so we cannot store usual TArray<> inside
    // this class solves the issue by removing copy constructors/operators from TArray
    template <typename T>
    class TNoncopyableArray : public TArray<T>
    {
    public:
        TNoncopyableArray(TNoncopyableArray&&) = default;
        TNoncopyableArray() = default;
    };

    void PrepareBindings(TOwner* BaseView)
    {
        check(BaseView); // this is ensured by caller

        // native bindings first
        BindProperties();

        // blueprint bindings second
        for (UnrealMvvm_Impl::FViewModelPropertyIterator Iter(TViewModel::StaticClass(), false); Iter; ++Iter)
        {
            UFunction* Function = BaseView->FindFunction(Iter->GetProperty()->GetCallbackName());

            if (Function)
            {
                UnrealMvvm_Impl::FBindEntry& Entry = BindEntries.Emplace_GetRef(Iter->GetProperty());
                Entry.Handler.Emplace<UnrealMvvm_Impl::FBlueprintPropertyChangeHandler>(BaseView, Function);
            }
        }

        // add at least one bind to prevent this method from being called again
        if (BindEntries.Num() == 0)
        {
            UnrealMvvm_Impl::FBindEntry& Entry = BindEntries.Emplace_GetRef(nullptr);
            Entry.Handler.Emplace<UnrealMvvm_Impl::FNoopPropertyChangeHandler>();
        }
    }

    void OnChanged(const FViewModelPropertyBase* Property)
    {
        TViewModel* ViewModel = GetViewModel();
        for (auto& Bind : BindEntries)
        {
            if (Bind.Property == Property)
            {
                Bind.Handler->Invoke(ViewModel, Property);
            }
        }
    }

    void OnViewConstructedChanged(bool bConstructed)
    {
        TViewModel* ViewModel = GetViewModel();
        if (!ViewModel)
        {
            // if we have no ViewModel there is nothing to do
            return;
        }

        if (bConstructed)
        {
            // View is constructed (i.e. visible), start listening and update current state
            StartListening(GetPointerToOwnerObject(), ViewModel);
        }
        else
        {
            // View is no longer attached to anything, stop listening to ViewModel
            StopListening(ViewModel);
        }
    }

    void StartListening(TOwner* BaseView, TViewModel* InViewModel)
    {
        check(BaseView); // this is ensured by caller
        check(InViewModel); // this is ensured by caller

        if (BindEntries.Num() != 0)
        {
            SubscriptionHandle = InViewModel->Subscribe(UBaseViewModel::FPropertyChangedDelegate::FDelegate::CreateWeakLambda(BaseView, [this](const FViewModelPropertyBase* P) { OnChanged(P); }));

            for (auto& Bind : BindEntries)
            {
                Bind.Handler->Invoke(InViewModel, Bind.Property);
            }
        }
    }

    void StopListening(TViewModel* InViewModel)
    {
        check(InViewModel); // this is ensured by caller

        if (BindEntries.Num() != 0)
        {
            InViewModel->Unsubscribe(SubscriptionHandle);
        }
    }

    TOwner* GetPointerToOwnerObject()
    {
        return static_cast<TOwner*>(this);
    }

    const TOwner* GetPointerToOwnerObject() const
    {
        return static_cast<const TOwner*>(this);
    }

    TNoncopyableArray< UnrealMvvm_Impl::FBindEntry > BindEntries;
    FDelegateHandle SubscriptionHandle;
    static uint8 Registered;
};

template<typename TOwner, typename TViewModel>
uint8 TBaseView<TOwner, TViewModel>::Registered = UnrealMvvm_Impl::FViewModelRegistry::RegisterViewModelClass(&TOwner::StaticClass, &TViewModel::StaticClass);