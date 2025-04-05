// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/BaseViewModel.h"
#include "TestBaseViewModel.generated.h"

/*
 * This struct has nonempty copy constructor which is used by Blueprint.
 * And if not properly initialized prior to copying into, will crash
 */
USTRUCT(BlueprintType)
struct FTestStructWithCopyConstructor
{
    GENERATED_BODY()

public:
    // FText needs custom constructor for copy and for default initialization
    UPROPERTY(BlueprintReadWrite)
    FText TextData;
};

UCLASS()
class UNREALMVVMTESTS_API UTestBaseViewModel : public UBaseViewModel
{
    GENERATED_BODY()

    VM_PROP_AG_AS(int32, IntValue, public, public);
    VM_PROP_AG_AS(float, FloatValue, public, public);
    VM_PROP_AG_AS(FTestStructWithCopyConstructor, StructValue, public);

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

UCLASS()
class UNREALMVVMTESTS_API UTestDerivedViewModel : public UTestBaseViewModel
{
    GENERATED_BODY()

    VM_PROP_AG_AS(int32, DerivedIntValue, public);

public:
};
