// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/BaseViewModel.h"
#include "DerivedViewModel.generated.h"

UCLASS()
class UBaseClassViewModel : public UBaseViewModel
{
    GENERATED_BODY()

    VM_PROP_AG_AS(int32, BaseClassValue, public, public);
};

UCLASS()
class UDerivedClassViewModel : public UBaseClassViewModel
{
    GENERATED_BODY()

    VM_PROP_AG_AS(int32, DerivedClassValue, public, public);
};