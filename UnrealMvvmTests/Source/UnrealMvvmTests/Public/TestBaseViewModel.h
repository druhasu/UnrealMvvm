// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/BaseViewModel.h"
#include "TestBaseViewModel.generated.h"

UCLASS()
class UNREALMVVMTESTS_API UTestBaseViewModel : public UBaseViewModel
{
    GENERATED_BODY()

    VM_PROP_AG_AS(int32, IntValue, public, public);
    VM_PROP_AG_AS(float, FloatValue, public, public);

    VM_PROP_AG_AS(int32, NoPublicGetter, private, public);
    VM_PROP_AG_AS(int32, NoPublicSetter, public, private);

public:
    using UBaseViewModel::HasConnectedViews;

    TOptional<bool> LastSubscriptionStatus;

    void RaiseSingleChange()
    {
        RaiseChanged(IntValueProperty());
    }

    void RaiseMultipleChange()
    {
        RaiseChanged(IntValueProperty(), FloatValueProperty());
    }

protected:
    void SubscriptionStatusChanged(bool bHasConnectedViews)
    {
        LastSubscriptionStatus = bHasConnectedViews;
    }
};