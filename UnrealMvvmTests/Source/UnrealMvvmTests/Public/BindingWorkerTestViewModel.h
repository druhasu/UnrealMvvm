// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/BaseViewModel.h"

#include "BindingWorkerTestViewModel.generated.h"

UCLASS()
class UBindingWorkerViewModel_SecondChild : public UBaseViewModel
{
    GENERATED_BODY()

    VM_PROP_AG_AS(int32, IntValue, public);
};

UCLASS()
class UBindingWorkerViewModel_FirstChild : public UBaseViewModel
{
    GENERATED_BODY()

    VM_PROP_AG_AS(int32, IntValue, public);
    VM_PROP_AG_AS(TObjectPtr<UBindingWorkerViewModel_SecondChild>, Child, public);
};

UCLASS()
class UBindingWorkerViewModel_DerivedFirstChild : public UBindingWorkerViewModel_FirstChild
{
    GENERATED_BODY()

    VM_PROP_AG_AS(int32, AnotherIntValue, public);
};

UCLASS()
class UBindingWorkerViewModel_Root : public UBaseViewModel
{
    GENERATED_BODY()

    VM_PROP_AG_AS(int32, IntValue, public);
    VM_PROP_AG_AS(int32, MinIntValue, public);
    VM_PROP_AG_AS(int32, MaxIntValue, public);

    VM_PROP_AG_AS(UBindingWorkerViewModel_FirstChild*, Child, public);
};

UCLASS()
class UBindingWorkerViewModel_RootDerived : public UBindingWorkerViewModel_Root
{
    GENERATED_BODY()

    VM_PROP_AG_AS(UBindingWorkerViewModel_FirstChild*, AnotherChild, public);
};
