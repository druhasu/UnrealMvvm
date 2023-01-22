// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

// DO NOT include this header directly!
// Use #include "Mvvm/BaseView.h"

#include "Mvvm/Impl/BindEntry.h"

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

        static const bool Value = TIsSame<decltype(Test<T>(0)), FText>::Value;
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
}

template<typename TOwner, typename TProperty, typename TCallback>
void __BindImpl(TOwner* ThisPtr, TProperty* Property, TCallback&& Callback)
{
    using namespace UnrealMvvm_Impl;

    using ViewModelType = typename TOwner::ViewModelType;

    static_assert(TIsDerivedFrom<TProperty, FViewModelPropertyBase>::Value, "Property must be derived from FViewModelPropertyBase");
    static_assert(TIsDerivedFrom<ViewModelType, typename TProperty::FViewModelType>::Value, "Property must be declared in TOwner's ViewModel type");

    auto& BindEntries = ThisPtr->GetBindEntries();

    // warn caller if he tries to bind several callbacks to a single property
    // we call only first callback when property changes, and this may lead to some confusion.
    // better say it here, when binding stuff
    const bool bNotBound = ensureAlwaysMsgf(
        !BindEntries.FindByPredicate([Property](const auto& Entry) { return Entry.Property == Property; }),
        TEXT("You are trying to bind property that is already bound. Only first callback will be called"));

    if (bNotBound)
    {
        FBindEntry& BindEntry = BindEntries.Emplace_GetRef(Property);
        BindEntry.Handler.Emplace<TBindingPropertyChangeHandler<ViewModelType, typename TProperty::FValueType, TCallback>>(Forward<TCallback>(Callback));
    }
}

// Binds property to a lambda
template<typename TOwner, typename TProperty, typename TCallback>
typename TEnableIf< TIsInvocable<TCallback, typename TProperty::FValueType>::Value >::Type
Bind(TOwner* ThisPtr, TProperty* Property, TCallback&& Callback)
{
    __BindImpl(ThisPtr, Property, MoveTemp(Callback));
}

// Binds property to a method of TOwner
template<typename TOwner, typename TProperty, typename TMemberPtr>
typename TEnableIf< TIsMemberPointer<TMemberPtr>::Value >::Type
Bind(TOwner* ThisPtr, TProperty* Property, TMemberPtr Callback)
{
    __BindImpl(ThisPtr, Property, [ThisPtr, Callback](typename TProperty::FValueType V) { (ThisPtr->*Callback)(V); });
}

// Binds FText property to UTextBlock or any other class that has SetText method
template<typename TOwner, typename TProperty, typename TTextBlock>
typename TEnableIf<TIsSame<typename TProperty::FValueType, FText>::Value&& UnrealMvvm_Impl::THasSetText<TTextBlock>::Value>::Type
Bind(TOwner* ThisPtr, TProperty* Property, TTextBlock* Text)
{
    check(Text);
    __BindImpl(ThisPtr, Property, [Text](typename TProperty::FValueType V) { Text->SetText(V); });
}

// Binds FString property to UTextBlock or any other class that has SetText method
template<typename TOwner, typename TProperty, typename TTextBlock>
typename TEnableIf<TIsSame<typename TProperty::FValueType, FString>::Value&& UnrealMvvm_Impl::THasSetText<TTextBlock>::Value>::Type
Bind(TOwner* ThisPtr, TProperty* Property, TTextBlock* Text)
{
    check(Text);
    __BindImpl(ThisPtr, Property, [Text](typename TProperty::FValueType V) { Text->SetText(FText::FromString(V)); });
}

// Binds numeric property to UTextBlock or any other class that has SetText method
template<typename TOwner, typename TProperty, typename TTextBlock>
typename TEnableIf<UnrealMvvm_Impl::THasNumberToText<typename TProperty::FValueType>::Value&& UnrealMvvm_Impl::THasSetText<TTextBlock>::Value>::Type
Bind(TOwner* ThisPtr, TProperty* Property, TTextBlock* Text, const FNumberFormattingOptions* const Options = nullptr, const FCulturePtr& TargetCulture = nullptr)
{
    check(Text);
    if (Options)
    {
        __BindImpl(ThisPtr, Property, [Text, Options = *Options, TargetCulture](typename TProperty::FValueType V) { Text->SetText(FText::AsNumber(V, &Options, TargetCulture)); });
    }
    else
    {
        __BindImpl(ThisPtr, Property, [Text, TargetCulture](typename TProperty::FValueType V) { Text->SetText(FText::AsNumber(V, nullptr, TargetCulture)); });
    }
}