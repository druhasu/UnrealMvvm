// Copyright Andrei Sudarikov. All Rights Reserved.

#include "K2Node_SetViewModelPropertyValue.h"
#include "Mvvm/BaseViewModel.h"
#include "Mvvm/Impl/Property/ViewModelRegistry.h"
#include "ViewModelPropertyNodeHelper.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "Kismet2/CompilerResultsLog.h"
#include "EdGraphSchema_K2.h"

void UK2Node_SetViewModelPropertyValue::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
    Super::ExpandNode(CompilerContext, SourceGraph);

    FViewModelPropertyNodeHelper::SpawnGetSetPropertyValueNodes(FViewModelPropertyNodeHelper::SetPropertyValueFunctionName, CompilerContext, this, SourceGraph, ViewModelPropertyName);
}

void UK2Node_SetViewModelPropertyValue::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
    AddSpawners(ActionRegistrar, [](const UnrealMvvm_Impl::FViewModelPropertyReflection& PropertyInfo)
    {
        return PropertyInfo.Flags.HasPublicSetter;
    });
}

void UK2Node_SetViewModelPropertyValue::AllocateDefaultPins()
{
    CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);
    CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);

    // create pin based on property type
    FEdGraphPinType PinType;
    const UnrealMvvm_Impl::FViewModelPropertyReflection* Property;
    if (FViewModelPropertyNodeHelper::FillPinType(PinType, ViewModelPropertyName, ViewModelOwnerClass, &Property))
    {
        CreatePin(EGPD_Input, PinType, ViewModelPropertyName);

        // create pin for TOptional HasValue
        if (Property->Flags.IsOptional)
        {
            UEdGraphPin* HasValuePin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Boolean, FViewModelPropertyNodeHelper::HasValuePinName);
            HasValuePin->DefaultValue = TEXT("true");
        }
    }

    Super::AllocateDefaultPins();
}

void UK2Node_SetViewModelPropertyValue::ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const
{
    Super::ValidateNodeDuringCompilation(MessageLog);

    using namespace UnrealMvvm_Impl;

    const FViewModelPropertyReflection* PropertyInfo = FViewModelRegistry::FindProperty(ViewModelOwnerClass, ViewModelPropertyName);
    if (PropertyInfo && !PropertyInfo->Flags.HasPublicSetter)
    {
        FText Message = FText::Format(
            NSLOCTEXT("UnrealMvvm", "Error.PropertyHasNoSetter", "@@ - Property {0}.{1} does not have public Setter"),
            FText::FromString(ViewModelOwnerClass->GetName()),
            FText::FromName(ViewModelPropertyName)
        );

        MessageLog.Error(*Message.ToString(), this);
    }
}

FText UK2Node_SetViewModelPropertyValue::GetNodeTitleForCache(ENodeTitleType::Type TitleType) const
{
    return TitleType == ENodeTitleType::FullTitle
        ? FText::Format(NSLOCTEXT("UnrealMvvm", "SetViewModelPropertyValue_TitleFull", "Set {0}\nTarget is {1}"), FText::FromName(ViewModelPropertyName), GetViewModelDisplayName())
        : FText::Format(NSLOCTEXT("UnrealMvvm", "SetViewModelPropertyValue_Title", "Set {0}"), FText::FromName(ViewModelPropertyName));
}

FText UK2Node_SetViewModelPropertyValue::GetTooltipTextForCache() const
{
    return FText::Format(NSLOCTEXT("UnrealMvvm", "SetViewModelPropertyValue_Tooltip", "Changes the value of property {0}"), FText::FromName(ViewModelPropertyName));
}
