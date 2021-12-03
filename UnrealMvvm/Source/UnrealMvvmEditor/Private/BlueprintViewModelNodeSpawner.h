// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "BlueprintNodeSpawner.h"
#include "BlueprintViewModelNodeSpawner.generated.h"

/*
 * Node spawner for K2Node_ViewModelPropertyValue.
 * Handles creation of a node with preset ViewModel class and property name
 */
UCLASS()
class UBlueprintViewModelNodeSpawner : public UBlueprintNodeSpawner
{
    GENERATED_BODY()

public:
    static UBlueprintViewModelNodeSpawner* CreateForProperty(TSubclassOf<UK2Node> NodeClass, UClass* ViewModelClass, const FName& ViewModelPropertyName);

    // UBlueprintNodeSpawner interface
    FBlueprintNodeSignature GetSpawnerSignature() const override;
    UEdGraphNode* Invoke(UEdGraph* ParentGraph, FBindingSet const& Bindings, FVector2D const Location) const override;
    FBlueprintActionUiSpec GetUiSpec(FBlueprintActionContext const& Context, FBindingSet const& Bindings) const override;
    // End UBlueprintNodeSpawner interface

private:
    UPROPERTY()
    UClass* ViewModelClass;
    FName ViewModelPropertyName;
};