// Copyright Andrei Sudarikov. All Rights Reserved.

#include "K2Node_GetViewModelPropertyValue.h"
#include "Mvvm/BaseViewModel.h"
#include "Mvvm/Impl/ViewModelPropertyIterator.h"
#include "ViewModelPropertyNodeHelper.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "EditorCategoryUtils.h"
#include "Kismet2/CompilerResultsLog.h"
#include "Kismet2/BlueprintEditorUtils.h"

void UK2Node_GetViewModelPropertyValue::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
    Super::ExpandNode(CompilerContext, SourceGraph);

    if (FindPin(ViewModelPropertyName)->LinkedTo.Num() > 0)
    {
        FName FunctionName = FViewModelPropertyNodeHelper::GetFunctionNameForGetPropertyValue(FBlueprintEditorUtils::FindBlueprintForNodeChecked(this)->GeneratedClass);
        FViewModelPropertyNodeHelper::SpawnGetSetPropertyValueNodes(FunctionName, CompilerContext, this, SourceGraph, ViewModelPropertyName);
    }
}

void UK2Node_GetViewModelPropertyValue::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
    AddSpawners(ActionRegistrar, [](const UnrealMvvm_Impl::FViewModelPropertyReflection& PropertyInfo)
    {
        return PropertyInfo.Flags.HasPublicGetter;
    });
}

void UK2Node_GetViewModelPropertyValue::AllocateDefaultPins()
{
    // create pin based on property type
    FEdGraphPinType PinType;
    const UnrealMvvm_Impl::FViewModelPropertyReflection* Property;
    if (FViewModelPropertyNodeHelper::FillPinType(PinType, ViewModelPropertyName, ViewModelOwnerClass, &Property))
    {
        CreatePin(EGPD_Output, PinType, ViewModelPropertyName);

        // create pin for TOptional HasValue
        if (Property->Flags.IsOptional)
        {
            CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Boolean, FViewModelPropertyNodeHelper::HasValuePinName);
        }
    }
}

void UK2Node_GetViewModelPropertyValue::ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const
{
    Super::ValidateNodeDuringCompilation(MessageLog);

    using namespace UnrealMvvm_Impl;

    const FViewModelPropertyReflection* PropertyInfo = FViewModelRegistry::FindProperty(ViewModelOwnerClass, ViewModelPropertyName);
    if (PropertyInfo && !PropertyInfo->Flags.HasPublicGetter)
    {
        FText Message = FText::Format(
            NSLOCTEXT("UnrealMvvm", "Error.PropertyHasNoGetter", "@@ - Property {0}.{1} does not have public Getter"),
            FText::FromString(ViewModelOwnerClass->GetName()),
            FText::FromName(ViewModelPropertyName)
        );

        MessageLog.Error(*Message.ToString(), this);
    }
}

FText UK2Node_GetViewModelPropertyValue::GetNodeTitleForCache(ENodeTitleType::Type TitleType) const
{
    return TitleType == ENodeTitleType::FullTitle
        ? FText::Format(NSLOCTEXT("UnrealMvvm", "GetViewModelPropertyValue_TitleFull", "Get {0}\nTarget is {1}"), FText::FromName(ViewModelPropertyName), GetViewModelDisplayName())
        : FText::Format(NSLOCTEXT("UnrealMvvm", "GetViewModelPropertyValue_Title", "Get {0}"), FText::FromName(ViewModelPropertyName));
}

FText UK2Node_GetViewModelPropertyValue::GetTooltipTextForCache() const 
{
    return FText::Format(NSLOCTEXT("UnrealMvvm", "GetViewModelPropertyValue_Tooltip", "Returns the value of property {0}"), FText::FromName(ViewModelPropertyName));
}