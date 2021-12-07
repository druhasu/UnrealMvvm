// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/ViewModelProperty.h"
#include "Mvvm/BaseViewModel.h"
#include "Mvvm/Impl/ViewModelRegistry.h"
#include "Mvvm/Impl/BindImpl.h"
#include "Templates/UnrealTypeTraits.h"
#include "Templates/Function.h"
#include "Templates/EnableIf.h"

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
        TViewModel* ViewModel = (TViewModel*)BaseView->ViewModel;

        if (ViewModel && BaseView->IsConstructed())
        {
            StopListening(ViewModel);
        }

        BaseView->ViewModel = InViewModel;

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
    virtual void BindProperties() {};

private:
    template<typename T, typename P, typename C>
    friend void __BindImpl(T*, P*, C&&);

    struct FBindEntry
    {
        const FViewModelPropertyBase* Property;
        TFunction<void(TViewModel*)> Callback;
    };

    void PrepareBindings(TOwner* BaseView)
    {
        check(BaseView); // this is ensured by caller

        // native bindings first
        BindProperties();

        // blueprint bindings second
        for (const UnrealMvvm_Impl::FViewModelPropertyReflection& PropertyInfo : UnrealMvvm_Impl::FViewModelRegistry::GetProperties<TViewModel>())
        {
            UFunction* Function = BaseView->FindFunction(PropertyInfo.Property->GetCallbackName());

            if (Function)
            {
                FBindEntry& Bind = BindEntries.AddDefaulted_GetRef();
                Bind.Property = PropertyInfo.Property;
                Bind.Callback = [BaseView, Function](TViewModel*)
                {
                    BaseView->ProcessEvent(Function, nullptr);
                };
            }
        }

        // add at least one bind to prevent this method from being called again
        if (BindEntries.Num() == 0)
        {
            BindEntries.Add(FBindEntry{ nullptr, [](auto){} });
        }
    }

    void OnChanged(const FViewModelPropertyBase* Property)
    {
        TViewModel* ViewModel = GetViewModel();
        for (auto& Bind : BindEntries)
        {
            if (Bind.Property == Property)
            {
                Bind.Callback(ViewModel);
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
                Bind.Callback(InViewModel);
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

    TOwner* GetPointerToOwnerObject() const
    {
        // we need this ugly hack to access ViewModel member of "unrelated" class UBaseView =(
        // we know exact class of this object - TOwner
        // we also know that we are a UObject
        // so we have a CDO. and its memory layout is same as ours
        // so we can find offset from "this" pointer to beginning of TOwner
        static UPTRINT PointerOffset = []()
        {
            using ThisType = TBaseView<TOwner, TViewModel>;
            const TOwner* DefaultObject = GetDefault<TOwner>();
            const ThisType* ThisObject = StaticCast<const ThisType*>(DefaultObject);
            auto Result = (UPTRINT)(void*)(ThisObject)-(UPTRINT)(void*)(DefaultObject);
            return Result;
        }();

        // use computed offset to find actual locationf of TOwner
        return (TOwner*)(((UPTRINT)(void*)this) - PointerOffset);
    }

    TArray<FBindEntry> BindEntries;
    FDelegateHandle SubscriptionHandle;
    static uint8 Registered;
};

template<typename TOwner, typename TViewModel>
uint8 TBaseView<TOwner, TViewModel>::Registered = UnrealMvvm_Impl::FViewModelRegistry::RegisterViewModelClass(&TOwner::StaticClass, &TViewModel::StaticClass);