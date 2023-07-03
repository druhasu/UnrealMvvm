// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Engine/DynamicBlueprintBinding.h"
#include "ViewModelDynamicBinding.generated.h"

UCLASS()
class UNREALMVVM_API UViewModelDynamicBinding : public UDynamicBlueprintBinding
{
    GENERATED_BODY()

public:
    void Serialize(FArchive& Ar) override;

    UPROPERTY()
    UClass* ViewModelClass;
};