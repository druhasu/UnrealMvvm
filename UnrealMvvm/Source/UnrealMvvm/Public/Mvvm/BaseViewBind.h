// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/ViewModelProperty.h"
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

        if (ViewModel && BindEntries.Num())
        {
            ViewModel->Changed.Remove(SubscriptionHandle);
        }

        BaseView->ViewModel = InViewModel;

        if (InViewModel)
        {
            if (BindEntries.Num() == 0)
            {
                BindProperties();
            }

            if (BindEntries.Num() != 0)
            {
                SubscriptionHandle = InViewModel->Changed.AddLambda([this](const FViewModelPropertyBase* P) { OnChanged(P); });

                for (auto& Bind : BindEntries)
                {
                    Bind.Callback(InViewModel);
                }
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

    typename TOwner::FBindEntry Bind;
    Bind.Property = Property;
    Bind.Callback = [Property, Callback = MoveTemp(Callback)](ViewModelType* VM)
    {
        auto Value = Property->GetValue(VM);
        Callback(Value);
    };

    ThisPtr->BindEntries.Emplace(Bind);
}

template<typename TOwner, typename TProperty, typename TCallback>
typename TEnableIf< TIsInvocable<TCallback, typename TProperty::ValueType>::Value >::Type
    Bind(TOwner* ThisPtr, TProperty* Property, TCallback&& Callback)
{
    __BindImpl(ThisPtr, Property, MoveTemp(Callback));
}

template<typename TOwner, typename TProperty, typename TMemberPtr>
typename TEnableIf< TIsMemberPointer<TMemberPtr>::Value >::Type
    Bind(TOwner* ThisPtr, TProperty* Property, TMemberPtr Callback)
{
    __BindImpl(ThisPtr, Property, [ThisPtr, Callback](typename TProperty::ValueType V) { (ThisPtr->*Callback)(V); });
}