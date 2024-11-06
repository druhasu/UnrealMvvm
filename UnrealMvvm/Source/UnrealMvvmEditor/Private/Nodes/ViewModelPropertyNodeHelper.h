// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "UObject/NameTypes.h"
#include "Mvvm/Impl/Property/ViewModelPropertyReflection.h"

struct FEdGraphPinType;
class UClass;
class UEdGraphPin;
class FKismetCompilerContext;
class UEdGraphNode;
class UEdGraph;
class UK2Node_CallFunction;

/* Helper class that is used across different ViewModel related custom nodes*/
class FViewModelPropertyNodeHelper
{
public:
    static bool IsPropertyAvailableInBlueprint(const UnrealMvvm_Impl::FViewModelPropertyReflection& Property);

    /* Fills FEdGraphPinType struct according to a type of ViewModel property. Optionally returns property reflection data */
    static bool FillPinType(FEdGraphPinType& PinType, const FName& ViewModelPropertyName, UClass* ViewModelOwnerClass, const UnrealMvvm_Impl::FViewModelPropertyReflection** OutProperty = nullptr);

    /* Fills FEdGraphPinType struct according to a type of ViewModel property */
    static bool FillPinType(FEdGraphPinType& PinType, const UnrealMvvm_Impl::FViewModelPropertyReflection* Property);

    /* Converts EPinCategoryType to FName used in BP editor */
    static FName GetPinCategoryNameFromType(UnrealMvvm_Impl::EPinCategoryType CategoryType);

    /* Converts EPinCategoryType to a subcategory name */
    static FName GetPinSubCategoryNameFromType(UnrealMvvm_Impl::EPinCategoryType CategoryType);

    /* Spawn GetSelf -> GetViewModelFromWidget/GetViewModelFromActor nodes */
    static UK2Node_CallFunction* SpawnGetViewModelNodes(FKismetCompilerContext& CompilerContext, UEdGraphNode* SourceNode, UEdGraph* SourceGraph);

    /* Spawns intermediate node equivalent to Self -> GetViewModelPropertyValue(View, ViewModelPropertyName) and connects its output to a given ValuePin */
    static void SpawnGetSetPropertyValueNodes(const FName& FunctionName, FKismetCompilerContext& CompilerContext, UEdGraphNode* SourceNode, UEdGraph* SourceGraph, const FName& ViewModelPropertyName);

    /* Returns whether specific node Blueprint is compatible with given ViewModel */
    static bool IsBlueprintViewModelCompatible(const UEdGraphNode* Node, UClass* ViewModelClass);

    /* Calls provided Action over each property in PropertyPath */
    static void ForEachPropertyInPath(TArrayView<const FName> PropertyPath, UClass* ViewModelClass, TFunctionRef<void(UClass*, FName, const UnrealMvvm_Impl::FViewModelPropertyReflection*)> Action);

    /* Checks whether provided PropertyPath is valid in context of given ViewModel class*/
    static bool IsPropertyPathValid(TArrayView<const FName> PropertyPath, UClass* ViewModelClass);

    /* Pin Name for HasValue */
    static const FName HasValuePinName;

    /* Pin Name for ViewModel */
    static const FName ViewModelPinName;

    /* Pin Name for View */
    static const FName ViewPinName;

    /* Name of UFUNCTION for GetViewModelPropertyValue */
    static const FName GetPropertyValueFunctionName;

    /* Name of UFUNCTION for SetViewModelPropertyValue */
    static const FName SetPropertyValueFunctionName;
};
