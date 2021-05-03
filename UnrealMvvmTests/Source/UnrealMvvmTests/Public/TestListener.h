// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "TestListener.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTestDynamicDelegate);

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