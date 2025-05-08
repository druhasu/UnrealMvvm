// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

// DO NOT include this header directly!
// Use #include "Mvvm/BaseView.h"

#include "Mvvm/Impl/Binding/BindingWorker.h"
#include "Mvvm/Impl/Utils/VariadicHelpers.h"
#include "Containers/StaticArray.h"
#include "Templates/IsInvocable.h"
#include "Templates/IsMemberPointer.h"
#include <type_traits>

namespace UnrealMvvm_Impl
{
    // checks whether method T::SetText(FText) exists
    template <typename T>
    struct THasSetText
    {
        template<typename U> static decltype(&U::SetText) Test(U*);
        template<typename U> static char Test(...);

        static const bool Value = TIsInvocable< decltype(Test<T>(nullptr)), T&, FText >::Value;
    };

    // checks whether method FText::AsNumber(T) exists
    template <typename T>
    struct THasNumberToText
    {
        template<typename U> static decltype(FText::AsNumber(DeclVal<U>())) Test(int);
        template<typename U> static char Test(...);

        static const bool Value = std::is_same_v<decltype(Test<T>(0)), FText>;
    };

    template <typename TOwner, typename TValue, typename TCallback>
    struct TBindingPropertyChangeHandler : public IPropertyChangeHandler
    {
        TBindingPropertyChangeHandler(TCallback&& InCallback)
            : Callback(InCallback)
        {
        }

        void Invoke(UBaseViewModel* ViewModel, const FViewModelPropertyBase* Property) const override
        {
            auto CastedProperty = (TViewModelProperty<TOwner, TValue>*)Property;
            Callback(CastedProperty->GetValue((TOwner*)ViewModel));
        }

        TCallback Callback;
    };


    template <typename TViewModel, typename TValue, uint32 Size>
    struct TPropertyPath
    {
        using FViewModelType = TViewModel;
        using FValueType = TValue;

        constexpr TPropertyPath(std::initializer_list<const FViewModelPropertyBase*> Props)
        {
            const FViewModelPropertyBase* const* It = Props.begin();
            for (uint32 Index = 0; Index < Size; ++Index, ++It)
            {
                Properties[Index] = *It;
            }
        }

        TArrayView<const FViewModelPropertyBase*> ToArrayView() const
        {
            return MakeArrayView(const_cast<const FViewModelPropertyBase**>(Properties.GetData()), Size);
        }

        TStaticArray<const FViewModelPropertyBase*, Size> Properties;
    };


    template <typename T>
    struct TPropertyPathTraits;

    template <typename TViewModel, typename TValue>
    struct TPropertyPathTraits<const TViewModelProperty<TViewModel, TValue>*>
    {
        using FValueType = TValue;
    };

    template <typename TViewModel, typename TValue, uint32 Size>
    struct TPropertyPathTraits<TPropertyPath<TViewModel, TValue, Size>>
    {
        using FValueType = TValue;
    };

    template <typename T>
    using TPropertyValueType_T = typename TPropertyPathTraits<T>::FValueType;
}

template<typename TOwner, typename TPropertyPath, typename TCallback>
void __BindImpl(TOwner* ThisPtr, TPropertyPath PropertyPath, TCallback&& Callback)
{
    using namespace UnrealMvvm_Impl;
    using ViewModelType = typename TOwner::ViewModelType;

    if constexpr (TIsDerivedFrom<TJustType_T<TPropertyPath>, FViewModelPropertyBase>::Value)
    {
        // this is just a single property
        using TProperty = TJustType_T<TPropertyPath>;

        static_assert(TIsDerivedFrom<ViewModelType, typename TProperty::FViewModelType>::Value, "Property must be declared in TOwner's ViewModel type");

        ThisPtr->template EmplaceHandler<TBindingPropertyChangeHandler<ViewModelType, typename TProperty::FValueType, TCallback>>({ PropertyPath }, Forward<TCallback>(Callback));
    }
    else
    {
        ThisPtr->template EmplaceHandler<TBindingPropertyChangeHandler<ViewModelType, typename TPropertyPath::FValueType, TCallback>>(PropertyPath.ToArrayView(), Forward<TCallback>(Callback));
    }
}

// Binds property to a lambda
template<typename TOwner, typename TProperty, typename TCallback>
typename TEnableIf< TIsInvocable<TCallback, UnrealMvvm_Impl::TPropertyValueType_T<TProperty>>::Value >::Type
Bind(TOwner* ThisPtr, TProperty Property, TCallback&& Callback)
{
    __BindImpl(ThisPtr, Property, MoveTemp(Callback));
}

// Binds property to a method of TOwner
template<typename TOwner, typename TProperty, typename TMemberPtr>
typename TEnableIf< TIsMemberPointer<TMemberPtr>::Value >::Type
Bind(TOwner* ThisPtr, TProperty Property, TMemberPtr Callback)
{
    __BindImpl(ThisPtr, Property, [ThisPtr, Callback](UnrealMvvm_Impl::TPropertyValueType_T<TProperty> V) { (ThisPtr->*Callback)(V); });
}

// Binds FText property to UTextBlock or any other class that has SetText method
template<typename TOwner, typename TProperty, typename TTextBlock>
typename TEnableIf<std::is_same_v<UnrealMvvm_Impl::TPropertyValueType_T<TProperty>, FText> && UnrealMvvm_Impl::THasSetText<typename TRemovePointer<typename TRemoveObjectPointer<TTextBlock>::Type>::Type>::Value>::Type
Bind(TOwner* ThisPtr, TProperty Property, TTextBlock Text)
{
    check(Text || ThisPtr->IsTemplate());
    __BindImpl(ThisPtr, Property, [Text](UnrealMvvm_Impl::TPropertyValueType_T<TProperty> V) { Text->SetText(V); });
}

// Binds FString property to UTextBlock or any other class that has SetText method
template<typename TOwner, typename TProperty, typename TTextBlock>
typename TEnableIf<std::is_same_v<UnrealMvvm_Impl::TPropertyValueType_T<TProperty>, FString> && UnrealMvvm_Impl::THasSetText<typename TRemovePointer<typename TRemoveObjectPointer<TTextBlock>::Type>::Type>::Value>::Type
Bind(TOwner* ThisPtr, TProperty Property, TTextBlock Text)
{
    check(Text || ThisPtr->IsTemplate());
    __BindImpl(ThisPtr, Property, [Text](UnrealMvvm_Impl::TPropertyValueType_T<TProperty> V) { Text->SetText(FText::FromString(V)); });
}

// Binds numeric property to UTextBlock or any other class that has SetText method
template<typename TOwner, typename TProperty, typename TTextBlock>
typename TEnableIf<UnrealMvvm_Impl::THasNumberToText<UnrealMvvm_Impl::TPropertyValueType_T<TProperty>>::Value&& UnrealMvvm_Impl::THasSetText<typename TRemovePointer<typename TRemoveObjectPointer<TTextBlock>::Type>::Type>::Value>::Type
Bind(TOwner* ThisPtr, TProperty Property, TTextBlock Text, const FNumberFormattingOptions* const Options = nullptr, const FCulturePtr& TargetCulture = nullptr)
{
    check(Text || ThisPtr->IsTemplate());
    if (Options)
    {
        __BindImpl(ThisPtr, Property, [Text, Options = *Options, TargetCulture](UnrealMvvm_Impl::TPropertyValueType_T<TProperty> V) { Text->SetText(FText::AsNumber(V, &Options, TargetCulture)); });
    }
    else
    {
        __BindImpl(ThisPtr, Property, [Text, TargetCulture](UnrealMvvm_Impl::TPropertyValueType_T<TProperty> V) { Text->SetText(FText::AsNumber(V, nullptr, TargetCulture)); });
    }
}
