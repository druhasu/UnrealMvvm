// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "UObject/Object.h"
#include "Mvvm/ViewModelProperty.h"
#include "Mvvm/ViewModelPropertyTypeTraits.h"
#include "Mvvm/Impl/Property/CanCompareHelper.h"
#include "Mvvm/Impl/Property/PropertyTypeSelector.h"
#include "Mvvm/Impl/Property/ViewModelPropertyMacros.h"
#include "BaseViewModel.generated.h"

#ifndef UE_REQUIRES
#define UE_REQUIRES , TEMPLATE_REQUIRES
#endif

/*
 * Base class for ViewModels
 */ 
UCLASS(HideDropDown, BlueprintType)
class UNREALMVVM_API UBaseViewModel : public UObject
{
    GENERATED_BODY()

public:
    DECLARE_MULTICAST_DELEGATE_OneParam(FPropertyChangedDelegate, const FViewModelPropertyBase*);

    /* Subscribes to changes of this ViewModel */
    FDelegateHandle Subscribe(FPropertyChangedDelegate::FDelegate&& Callback)
    {
        if (!Changed.IsBound())
        {
            SubscriptionStatusChanged(true);
        }

        return Changed.Add(Callback);
    }

    /* Unsubscribes from changes of this ViewModel by DelegateHandle */
    void Unsubscribe(FDelegateHandle Handle)
    {
        const bool bWasBound = Changed.IsBound();
        Changed.Remove(Handle);

        if (bWasBound && !Changed.IsBound())
        {
            SubscriptionStatusChanged(false);
        }
    }

    /* Unsubscribes given Object from changes of this ViewModel */
    void Unsubscribe(const void* InUserObject)
    {
        const bool bWasBound = Changed.IsBound();
        Changed.RemoveAll(InUserObject);

        if (bWasBound && !Changed.IsBound())
        {
            SubscriptionStatusChanged(false);
        }
    }

protected:
    /* Call this method to notify any connected View that given property was changed */
    void RaiseChanged(const FViewModelPropertyBase* Property)
    {
        checkf(Property, TEXT("You should not call RaiseChanged with nullptr property"));
        Changed.Broadcast(Property);
    }

    /* Call this method to notify any connected View that given properties were changed */
    template <typename... TProperty UE_REQUIRES(sizeof...(TProperty) >= 2)>
    void RaiseChanged(const TProperty*... Props)
    {
        (RaiseChanged(Props), ...);
    }

    /*
     * Called when first subscription added or last subscription removed.
     * i.e. at least one View as listening to changes or no Views are listening to changes anymore.
     * 
     * You can use this method to optimize your ViewModel in situations when no Views are listening,
     * e.g. unsubscribe from some service notifications, prevent doing heavy operations, etc
     */
    virtual void SubscriptionStatusChanged(bool bHasConnectedViews) {}

    /* Returns whether this ViewModel has any Views listening to its changes */
    bool HasConnectedViews() const { return Changed.IsBound(); }

    /*
     * Sets new value to provided variable.
     * Optionaly performs comparison of current value and new value.
     * Returns true if value was changed
     */
    template <typename TValue>
    bool TrySetValue(TValue& Field, typename UnrealMvvm_Impl::TPropertyTypeSelector<TValue>::SetterType InValue)
    {
        // check if we need to compare values
        if constexpr (TViewModelPropertyTypeTraits<TValue>::WithSetterComparison && UnrealMvvm_Impl::TCanCompareHelper<TValue>::Value)
        {
            // check if we need to compare structs using Identical
            if constexpr (TStructOpsTypeTraits<TValue>::WithIdentical)
            {
                // use Identical method
                if (!Field.Identical(&InValue, 0))
                {
                    Field = InValue;
                    return true;
                }
                return false;
            }
            else
            {
                // use operator ==
                if (!(Field == InValue))
                {
                    Field = InValue;
                    return true;
                }
                return false;
            }
        }
        else
        {
            // no comparison needed, just set the value
            Field = InValue;
            return true;
        }
    }

private:
    FPropertyChangedDelegate Changed;
};
