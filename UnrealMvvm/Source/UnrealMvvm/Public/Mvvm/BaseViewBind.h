// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/ViewModelProperty.h"
#include "Mvvm/BaseViewModel.h"
#include "Templates/UnrealTypeTraits.h"
#include "Templates/Function.h"
#include "Templates/EnableIf.h"

template<typename TOwner, typename TViewModel>
class TBaseView
{
public:
    using ViewModelType = TViewModel;

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
                BindProperties();
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
    virtual void BindProperties() = 0;

private:
    template<typename T, typename P, typename C>
    friend void __BindImpl(T*, P*, C&&);

    struct FBindEntry
    {
        const FViewModelPropertyBase* Property;
        TFunction<void(TViewModel*)> Callback;
    };

    void OnChanged(const FViewModelPropertyBase* Property)
    {
        TViewModel* ViewModel = GetViewModel();
        for (auto& Bind : BindEntries)
        {
            if (Bind.Property == Property)
            {
                Bind.Callback(ViewModel);
                return;
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
};

template<typename TOwner, typename TProperty, typename TCallback>
void __BindImpl(TOwner* ThisPtr, TProperty* Property, TCallback&& Callback)
{
    using ViewModelType = typename TOwner::ViewModelType;

    static_assert(TIsDerivedFrom<TProperty, FViewModelPropertyBase>::Value, "Property must be derived from FViewModelPropertyBase");
    static_assert(TIsDerivedFrom<ViewModelType, typename TProperty::ViewModelType>::Value, "Property must be declared in TOwner's ViewModel type");

    // warn caller if he tries to bind several callbacks to a single property
    // we call only first callback when property changes, and this may lead to some confusion.
    // better say it here, when binding stuff
    const bool bNotBound = ensureAlwaysMsgf(
        !ThisPtr->BindEntries.FindByPredicate([Property](const auto& Entry) { return Entry.Property == Property; }),
        TEXT("You are trying to bind property that is already bound. Only first callback will be called"));

    if (bNotBound)
    {
        typename TOwner::FBindEntry Bind;
        Bind.Property = Property;
        Bind.Callback = [Property, Callback = MoveTemp(Callback)](ViewModelType* VM)
        {
            auto Value = Property->GetValue(VM);
            Callback(Value);
        };

        ThisPtr->BindEntries.Emplace(Bind);
    }
}

namespace ViewModelBind_Private
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
}

// Binds property to a lambda
template<typename TOwner, typename TProperty, typename TCallback>
typename TEnableIf< TIsInvocable<TCallback, typename TProperty::ValueType>::Value >::Type
Bind(TOwner* ThisPtr, TProperty* Property, TCallback&& Callback)
{
    __BindImpl(ThisPtr, Property, MoveTemp(Callback));
}

// Binds property to a method of TOwner
template<typename TOwner, typename TProperty, typename TMemberPtr>
typename TEnableIf< TIsMemberPointer<TMemberPtr>::Value >::Type
Bind(TOwner* ThisPtr, TProperty* Property, TMemberPtr Callback)
{
    __BindImpl(ThisPtr, Property, [ThisPtr, Callback](typename TProperty::ValueType V) { (ThisPtr->*Callback)(V); });
}

// Binds FText property to UTextBlock or any other class that has SetText method
template<typename TOwner, typename TProperty, typename TTextBlock>
typename TEnableIf<TIsSame<typename TProperty::ValueType, FText>::Value && ViewModelBind_Private::THasSetText<TTextBlock>::Value>::Type
Bind(TOwner* ThisPtr, TProperty* Property, TTextBlock* Text)
{
    check(Text);
    __BindImpl(ThisPtr, Property, [Text](typename TProperty::ValueType V) { Text->SetText(V); });
}

// Binds FString property to UTextBlock or any other class that has SetText method
template<typename TOwner, typename TProperty, typename TTextBlock>
typename TEnableIf<TIsSame<typename TProperty::ValueType, FString>::Value && ViewModelBind_Private::THasSetText<TTextBlock>::Value>::Type
Bind(TOwner* ThisPtr, TProperty* Property, TTextBlock* Text)
{
    check(Text);
    __BindImpl(ThisPtr, Property, [Text](typename TProperty::ValueType V) { Text->SetText(FText::FromString(V)); });
}

// Binds numeric property to UTextBlock or any other class that has SetText method
template<typename TOwner, typename TProperty, typename TTextBlock>
typename TEnableIf<ViewModelBind_Private::THasNumberToText<typename TProperty::ValueType>::Value && ViewModelBind_Private::THasSetText<TTextBlock>::Value>::Type
Bind(TOwner* ThisPtr, TProperty* Property, TTextBlock* Text, const FNumberFormattingOptions* const Options = nullptr, const FCulturePtr& TargetCulture = nullptr)
{
    check(Text);
    if (Options)
    {
        __BindImpl(ThisPtr, Property, [Text, Options = *Options, TargetCulture](typename TProperty::ValueType V) { Text->SetText(FText::AsNumber(V, &Options, TargetCulture)); });
    }
    else
    {
        __BindImpl(ThisPtr, Property, [Text, TargetCulture](typename TProperty::ValueType V) { Text->SetText(FText::AsNumber(V, nullptr, TargetCulture)); });
    }
}