// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "UObject/NameTypes.h"
#include "Mvvm/Impl/ViewModelPropertyReflection.h"

struct FEdGraphPinType;
class UClass;
class UEdGraphPin;
class FKismetCompilerContext;
class UEdGraphNode;
class UEdGraph;

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

    /* Connects input ViewModel to GetViewModelPropertyValue(ViewModel, ViewModelPropertyName) and connects its output to a given ValuePin */
    static void SpawnExplicitGetSetPropertyValueNodes(const FName& FunctionName, FKismetCompilerContext& CompilerContext, UEdGraphNode* SourceNode, UEdGraph* SourceGraph, const FName& ViewModelPropertyName);

    /* Adds an input ViewModel pin to the getter/setter node. */
    static void AddInputViewModelPin(UEdGraphNode& Node, UClass* ViewModelClass);

    /* Returns true if an action for the given ViewModelOwnerClass is valid for current context represented by Filter and should be shown in actions menu. */
    static bool IsActionValidInContext(const FBlueprintActionFilter& Filter, const UClass* ViewModelOwnerClass);

    static void ValidateInputViewModelPin(const UEdGraphNode* Node, FCompilerResultsLog& MessageLog);

    static FName GetFunctionNameForGetPropertyValue(UClass* ViewClass);
    static FName GetFunctionNameForSetPropertyValue(UClass* ViewClass);
    static FName GetFunctionNameForGetPropertyValue();
    static FName GetFunctionNameForSetPropertyValue();

    /* Pin Name for HasValue */
    static const FName HasValuePinName;

    /* Pin Name for input ViewModel */
    static const FName InputViewModelPinName;

private:
    static void ConnectOutputPins(class UK2Node_CallFunction* GetSetFunctionNode, FKismetCompilerContext& CompilerContext, UEdGraphNode* SourceNode, UEdGraph* SourceGraph, const UEdGraphSchema_K2* Schema, UEdGraphPin* ValuePin, const FName& ViewModelPropertyName);
};