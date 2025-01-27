// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/ViewModelProperty.h"
#include "Mvvm/Impl/Binding/BindImpl.h"
#include "Mvvm/Impl/BaseView/BaseViewImpl.h"
#include "Mvvm/Impl/BaseView/ViewRegistry.h"
#include "Mvvm/Impl/Utils/VariadicHelpers.h"

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
        return UnrealMvvm_Impl::TBaseViewImpl<TOwner, TViewModel>::GetViewModel(this);
    }

    void SetViewModel(TViewModel* InViewModel)
    {
        UnrealMvvm_Impl::TBaseViewImpl<TOwner, TViewModel>::SetViewModel(this, InViewModel);
    }

protected:
    /* Override this method to setup bindings. You should not call anything except Bind from it */
    virtual void BindProperties() {}

    /* Override this method to get notified about ViewModel changes. OldViewModel and NewViewModel may be nullptr */
    virtual void OnViewModelChanged(TViewModel* OldViewModel, TViewModel* NewViewModel) {}

    /* Creates Property Path for passing into Bind function */
    template
    <
        typename... TProps,
        uint32 Size = sizeof...(TProps),
        typename TValue = typename UnrealMvvm_Impl::TLastType_T<typename UnrealMvvm_Impl::TJustType_T<TProps>...>::FValueType
    >
    constexpr UnrealMvvm_Impl::TPropertyPath<TViewModel, TValue, Size> Path(TProps&&... Props)
    {
        UnrealMvvm_Impl::TPropertyPathValidator<TViewModel, TProps...>::Validate();
        return { { Props... } };
    }

private:
    template<typename T, typename P, typename C>
    friend void __BindImpl(T*, P, C&&);

    template<typename O, typename V, typename U>
    friend class UnrealMvvm_Impl::TBaseViewImplWithComponent;

    template<typename O, typename V, typename U>
    friend class UnrealMvvm_Impl::TBaseViewImpl;

    static void SetViewModelStatic(UObject& ViewObject, UBaseViewModel* ViewModel)
    {
        TOwner& Owner = static_cast<TOwner&>(ViewObject);
        TBaseView<TOwner, TViewModel>& TypedView = static_cast<TBaseView<TOwner, TViewModel>&>(Owner);

        TypedView.SetViewModel((TViewModel*)ViewModel);
    }

    static void CollectNativeBindings(UObject& ViewObject)
    {
        TOwner& Owner = static_cast<TOwner&>(ViewObject);
        TBaseView<TOwner, TViewModel>& TypedView = static_cast<TBaseView<TOwner, TViewModel>&>(Owner);

        TypedView.BindProperties();
    }

    template <typename THandler, typename... TArgs>
    void EmplaceHandler(TArrayView<const FViewModelPropertyBase* const> PropertyPath, TArgs&&... Args)
    {
        using namespace UnrealMvvm_Impl;

        if (!FViewRegistry::RecordPropertyPath(PropertyPath))
        {
            FBindingWorker& Worker = TBaseViewImpl<TOwner, TViewModel>::GetBindingWorker(this);
            Worker.AddBindingHandler<THandler, TArgs...>(PropertyPath, Forward<TArgs>(Args)...);
        }
    }

    UObject* CachedComponent = nullptr;
    static uint8 Registered;
};

template<typename TOwner, typename TViewModel>
uint8 TBaseView<TOwner, TViewModel>::Registered = UnrealMvvm_Impl::FViewRegistry::RegisterViewClass(&TOwner::StaticClass, &TViewModel::StaticClass, &TBaseView<TOwner, TViewModel>::SetViewModelStatic, &TBaseView<TOwner, TViewModel>::CollectNativeBindings);
