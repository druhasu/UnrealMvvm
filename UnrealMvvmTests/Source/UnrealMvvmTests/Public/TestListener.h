// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Templates/SharedPointer.h"
#include "TestListener.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTestDynamicDelegate);

struct FTestListener : public TSharedFromThis<FTestListener>
{
public:
    void SimpleCallback() { Invoked = true; }

    bool Invoked = false;
};

UCLASS()
class UTestListener : public UObject
{
    GENERATED_BODY()

public:
    void SimpleCallback() { Invoked = true; }

    UFUNCTION()
    void DynamicCallback() { Invoked = true; }

    bool Invoked = false;
};

UCLASS()
class UTestEventHolder : public UObject
{
    GENERATED_BODY()

public:
    DECLARE_MULTICAST_DELEGATE(FTestDelegate);
    FTestDelegate DelegateField;

    FTestDynamicDelegate DynamicDelegateField;
};
