// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/ViewModelProperty.h"
#include "Mvvm/Impl/BaseView/BaseViewExtension.h"
#include "Mvvm/Impl/BaseView/BaseViewComponent.h"

template<typename TOwner, typename TViewModel>
class TBaseView;

namespace UnrealMvvm_Impl
{
    template<typename TOwner, typename TViewModel, typename TComponent>
    class TBaseViewImplWithComponent
    {
    public:
        using FView = TBaseView<TOwner, TViewModel>;

        static TViewModel* GetViewModel(const FView* BaseView)
        {
            return (TViewModel*)GetExtension(BaseView)->ViewModel;
        }

        static void SetViewModel(FView* BaseView, TViewModel* InViewModel)
        {
            TComponent* Extension = GetExtension(BaseView);

            TViewModel* OldViewModel = (TViewModel*)Extension->ViewModel;

            Extension->SetViewModelInternal(InViewModel);
            BaseView->OnViewModelChanged(OldViewModel, InViewModel);
        }

        static TComponent* GetExtension(const FView* BaseView)
        {
            if (!BaseView->CachedComponent)
            {
                const_cast<FView*>(BaseView)->CachedComponent = TComponent::Request(const_cast<TOwner*>(static_cast<const TOwner*>(BaseView)));
            }

            return (TComponent*)BaseView->CachedComponent;
        }

        static auto& GetBindingWorker(const FView* BaseView)
        {
            return GetExtension(BaseView)->BindingWorker;
        }
    };

    template<typename TOwner, typename TViewModel, typename = void>
    class TBaseViewImpl;

    /* Implementation for UserWidget */
    template<typename TOwner, typename TViewModel>
    class TBaseViewImpl<TOwner, TViewModel, typename TEnableIf<TIsDerivedFrom<TOwner, UUserWidget>::Value>::Type>
        : public TBaseViewImplWithComponent<TOwner, TViewModel, UBaseViewExtension>
    {
    };

    /* Implementation for Actor */
    template<typename TOwner, typename TViewModel>
    class TBaseViewImpl<TOwner, TViewModel, typename TEnableIf<TIsDerivedFrom<TOwner, AActor>::Value>::Type>
        : public TBaseViewImplWithComponent<TOwner, TViewModel, UBaseViewComponent>
    {
    };

    template <typename TResultType, typename... TProps>
    struct TPropertyPathValidator;

    template <typename TResultType, typename TProp, typename... TOtherProps>
    struct TPropertyPathValidator<TResultType, TProp, TOtherProps...>
    {
        static constexpr void Validate()
        {
            using FDecayedProperty = TJustType_T<TProp>;

            /*
              If you hit this assert then your binding path is not valid.

              Check following:
                - first property in path belongs to ViewModelType
                - each next property belongs to a ViewModel returned by previous.
                  For example, previous property type is TViewModelProperty<ViewModelType, UChildViewModel*>.
                  Then, next property must be TViewModelProperty<UChildViewModel, ... >
             */
            static_assert(std::is_same_v< TResultType, typename FDecayedProperty::FViewModelType >, "TResultType must be the same as owner of TProp");

            if constexpr (sizeof...(TOtherProps) > 0)
            {
                TPropertyPathValidator
                <
                    typename TRemoveObjectPointer
                    <
                        std::remove_pointer_t< typename FDecayedProperty::FValueType >
                    >::Type,
                    TOtherProps...
                >::Validate();
            }
        }
    };
}
