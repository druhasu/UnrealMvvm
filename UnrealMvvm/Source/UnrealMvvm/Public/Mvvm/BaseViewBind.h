// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/ViewModelProperty.h"
#include "Mvvm/Impl/BindImpl.h"
#include "Mvvm/Impl/BaseViewExtension.h"

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
        return (TViewModel*)GetExtension()->ViewModel;
    }

    void SetViewModel(TViewModel* InViewModel)
    {
        UBaseViewExtension* Extension = GetExtension();

        if (Extension->BindEntries.Num() == 0)
        {
            BindProperties();
            Extension->PrepareBindings(TViewModel::StaticClass());
        }

        TViewModel* OldViewModel = (TViewModel*)Extension->ViewModel;

        Extension->SetViewModelInternal(InViewModel);
        OnViewModelChanged(OldViewModel, InViewModel);
    }

protected:
    /* Override this method to setup bindings. You should not call anything except Bind from it */
    virtual void BindProperties() {}

    /* Override this method to get notified about ViewModel changes. OldViewModel and NewViewModel may be nullptr */
    virtual void OnViewModelChanged(TViewModel* OldViewModel, TViewModel* NewViewModel) {}

private:
    template<typename T, typename P, typename C>
    friend void __BindImpl(T*, P*, C&&);

    static void SetViewModelStatic(UUserWidget& Widget, UBaseViewModel* ViewModel)
    {
        TOwner& Owner = static_cast<TOwner&>(Widget);
        TBaseView<TOwner, TViewModel>& TypedView = static_cast<TBaseView<TOwner, TViewModel>&>(Owner);

        Owner.SetViewModel((TViewModel*)ViewModel);
    }

    UBaseViewExtension* GetExtension() const
    {
        if (!CachedExtension)
        {
            CachedExtension = UBaseViewExtension::Request(static_cast<const TOwner*>(this));
        }

        return CachedExtension;
    }

    auto& GetBindEntries()
    {
        return GetExtension()->BindEntries;
    }

    mutable UBaseViewExtension* CachedExtension = nullptr;
    static uint8 Registered;
};

template<typename TOwner, typename TViewModel>
uint8 TBaseView<TOwner, TViewModel>::Registered = UnrealMvvm_Impl::FViewModelRegistry::RegisterViewClass(&TOwner::StaticClass, &TViewModel::StaticClass, &TBaseView<TOwner, TViewModel>::SetViewModelStatic);