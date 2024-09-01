// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/BaseViewModel.h"

#include "BindingWorkerTestViewModel.generated.h"

UCLASS(HideDropDown)
class UBindingWorkerViewModel_SecondChild : public UBaseViewModel
{
    GENERATED_BODY()

    VM_PROP_AG_AS(int32, IntValue, public);
};

UCLASS(HideDropDown)
class UBindingWorkerViewModel_FirstChild : public UBaseViewModel
{
    GENERATED_BODY()

    VM_PROP_AG_AS(int32, IntValue, public);
    VM_PROP_AG_AS(UBindingWorkerViewModel_SecondChild*, Child, public);
};

UCLASS(HideDropDown)
class UBindingWorkerViewModel_Root : public UBaseViewModel
{
    GENERATED_BODY()

    VM_PROP_AG_AS(int32, IntValue, public);
    VM_PROP_AG_AS(int32, MinIntValue, public);
    VM_PROP_AG_AS(int32, MaxIntValue, public);

    VM_PROP_AG_AS(UBindingWorkerViewModel_FirstChild*, Child, public);
};
