// Copyright Andrei Sudarikov. All Rights Reserved.

#include "K2Node_ExplicitSetViewModelPropertyValue.h"
#include "ViewModelPropertyNodeHelper.h"
#include "Kismet2/CompilerResultsLog.h"

void UK2Node_ExplicitSetViewModelPropertyValue::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
    FViewModelPropertyNodeHelper::SpawnExplicitGetSetPropertyValueNodes(FViewModelPropertyNodeHelper::GetFunctionNameForSetPropertyValue(), CompilerContext, this, SourceGraph, ViewModelPropertyName);
}

FText UK2Node_ExplicitSetViewModelPropertyValue::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
    FText Title = Super::GetNodeTitle(TitleType);
    if (TitleType == ENodeTitleType::MenuTitle || TitleType == ENodeTitleType::ListView)
    {
        Title = FText::Format(NSLOCTEXT("UnrealMvvm", "ExplicitSetViewModelPropertyValue_Title", "{0} (explicit)"), Title);
    }
    return Title;
}

bool UK2Node_ExplicitSetViewModelPropertyValue::IsActionFilteredOut(FBlueprintActionFilter const& Filter)
{
    return !FViewModelPropertyNodeHelper::IsActionValidInContext(Filter, ViewModelOwnerClass);
}

void UK2Node_ExplicitSetViewModelPropertyValue::AllocateDefaultPins()
{
    Super::AllocateDefaultPins();

    FViewModelPropertyNodeHelper::AddInputViewModelPin(*this, ViewModelOwnerClass);
}

void UK2Node_ExplicitSetViewModelPropertyValue::ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const
{
    Super::ValidateNodeDuringCompilation(MessageLog);

    FViewModelPropertyNodeHelper::ValidateInputViewModelPin(this, MessageLog);
}

FText UK2Node_ExplicitSetViewModelPropertyValue::GetNodeTitleForCache(ENodeTitleType::Type TitleType) const
{
    return TitleType == ENodeTitleType::FullTitle
        ? FText::Format(NSLOCTEXT("UnrealMvvm", "ExplicitSetViewModelPropertyValue_TitleFull", "Set {0}\nTo {1}"), FText::FromName(ViewModelPropertyName), GetViewModelDisplayName())
        : FText::Format(NSLOCTEXT("UnrealMvvm", "ExplicitSetViewModelPropertyValue_Title", "Set {0}"), FText::FromName(ViewModelPropertyName));
}
