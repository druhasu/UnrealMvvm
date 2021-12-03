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

    /* Fills FEdGraphPinType struct according to a type of ViewModel property*/
    static bool FillPinType(FEdGraphPinType& PinType, const FName& ViewModelPropertyName, UClass* ViewModelOwnerClass);

    /* Converts EPinCategoryType to FName used in BP editor */
    static FName GetPinCategoryNameFromType(UnrealMvvm_Impl::EPinCategoryType CategoryType);

    /* Spawns intermediate node equivalent to Self -> GetViewModelPropertyValue(View, ViewModelPropertyName) and connects its output to a given ValuePin */
    static void SpawnReadPropertyValueNodes(UEdGraphPin* ValuePin, FKismetCompilerContext& CompilerContext, UEdGraphNode* SourceNode, UEdGraph* SourceGraph, const FName& ViewModelPropertyName);
};