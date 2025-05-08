// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Engine/DynamicBlueprintBinding.h"
#include "ViewModelDynamicBinding.generated.h"

USTRUCT()
struct FBlueprintBindingEntry
{
    GENERATED_BODY()

public:
    UPROPERTY()
    TArray<FName> PropertyPath;

    UPROPERTY()
    FName FunctionName;
};

UCLASS()
class UNREALMVVM_API UViewModelDynamicBinding : public UDynamicBlueprintBinding
{
    GENERATED_BODY()

public:
    void PostLoad() override;

    UPROPERTY()
    TObjectPtr<UClass> ViewModelClass;

    UPROPERTY()
    TArray<FBlueprintBindingEntry> BlueprintBindings;
};
