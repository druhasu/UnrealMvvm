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
class UK2Node_ViewModelPropertyBase;

/* Helper class that is used across different ViewModel related custom nodes*/
class FViewModelPropertyNodeHelper
{
public:
    static bool IsPropertyAvailableInBlueprint(const UnrealMvvm_Impl::FViewModelPropertyReflection& Property);

    /* Fills FEdGraphPinType struct according to a type of ViewModel property. Optionally returns property reflection data */
    static bool FillPinType(FEdGraphPinType& PinType, const FName& ViewModelPropertyName, UClass* ViewModelOwnerClass, const UnrealMvvm_Impl::FViewModelPropertyReflection** OutProperty = nullptr);

    /* Converts EPinCategoryType to FName used in BP editor */
    static FName GetPinCategoryNameFromType(UnrealMvvm_Impl::EPinCategoryType CategoryType);

    /* Converts EPinCategoryType to a subcategory name */
    static FName GetPinSubCategoryNameFromType(UnrealMvvm_Impl::EPinCategoryType CategoryType);

    /* Spawns intermediate node equivalent to Self -> GetViewModelPropertyValue(View, ViewModelPropertyName) and connects its output to a given ValuePin */
    static void SpawnGetSetPropertyValueNodes(const FName& FunctionName, FKismetCompilerContext& CompilerContext, UEdGraphNode* SourceNode, UEdGraph* SourceGraph, const FName& ViewModelPropertyName);

    /* Adds an input ViewModel pin to the getter/setter node. */
    static void AddInputViewModelPin(UEdGraphNode& Node, UClass* ViewModelClass, bool bShowPin);

    /* Returns whether specific node Blueprint is compatible with given ViewModel */
    static bool IsBlueprintViewModelCompatible(const UEdGraphNode* Node, UClass* ViewModelClass);

    static FName GetFunctionNameForGetPropertyValue();
    static FName GetFunctionNameForSetPropertyValue();

    /* Pin Name for HasValue */
    static const FName HasValuePinName;

    /* Pin Name for ViewModel */
    static const FName ViewModelPinName;

    /* Pin Name for View */
    static const FName ViewPinName;

private:
    static void ConnectOutputPins(class UK2Node_CallFunction* GetSetFunctionNode, FKismetCompilerContext& CompilerContext, UEdGraphNode* SourceNode, UEdGraph* SourceGraph, const UEdGraphSchema_K2* Schema, UEdGraphPin* ValuePin, const FName& ViewModelPropertyName);
};
