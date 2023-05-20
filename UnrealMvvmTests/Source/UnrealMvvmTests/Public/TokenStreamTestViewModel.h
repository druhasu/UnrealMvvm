// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/BaseViewModel.h"
#include "TokenStreamTestViewModel.generated.h"

UCLASS()
class UTokenStreamTestViewModel : public UBaseViewModel
{
    GENERATED_BODY()

    VM_PROP_AG_AS(UObject*, Pointer, public, public);
    VM_PROP_AG_AS((TMap<UObject*, int32>), PointerMap, public, public);
    VM_PROP_AG_AS(TSet<UObject*>, PointerSet, public, public);
};

UCLASS()
class UTokenStreamTargetClass_NoProperties : public UObject
{
    GENERATED_BODY()
};

UCLASS()
class UTokenStreamTargetClass_WithProperties : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY()
    FString FirstProperty;
};