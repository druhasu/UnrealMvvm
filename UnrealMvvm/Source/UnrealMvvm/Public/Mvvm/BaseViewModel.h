// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/ViewModelProperty.h"
#include "Mvvm/Impl/PropertyTypeSelector.h"
#include "Mvvm/Impl/ViewModelPropertyMacros.h"
#include "CoreMinimal.h"
#include "BaseViewModel.generated.h"

/*
 * Base class for ViewModels
 */ 
UCLASS()
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
        Changed.Broadcast(Property);
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

private:
    FPropertyChangedDelegate Changed;
};
