// Copyright Andrei Sudarikov. All Rights Reserved.

#include "K2Node_ExplicitGetViewModelPropertyValue.h"
#include "ViewModelPropertyNodeHelper.h"
#include "Kismet2/CompilerResultsLog.h"

void UK2Node_ExplicitGetViewModelPropertyValue::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
    if (FindPin(ViewModelPropertyName)->LinkedTo.Num() > 0)
    {
        FViewModelPropertyNodeHelper::SpawnExplicitGetSetPropertyValueNodes(FViewModelPropertyNodeHelper::GetFunctionNameForGetPropertyValue(), CompilerContext, this, SourceGraph, ViewModelPropertyName);
    }
}

FText UK2Node_ExplicitGetViewModelPropertyValue::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
    FText Title = Super::GetNodeTitle(TitleType);
    if (TitleType == ENodeTitleType::MenuTitle || TitleType == ENodeTitleType::ListView)
    {
        Title = FText::Format(NSLOCTEXT("UnrealMvvm", "ExplicitGetViewModelPropertyValue_Title", "{0} (explicit)"), Title);
    }
    return Title;
}

bool UK2Node_ExplicitGetViewModelPropertyValue::IsActionFilteredOut(FBlueprintActionFilter const& Filter)
{
    return !FViewModelPropertyNodeHelper::IsActionValidInContext(Filter, ViewModelOwnerClass);
}

void UK2Node_ExplicitGetViewModelPropertyValue::AllocateDefaultPins()
{
    Super::AllocateDefaultPins();

    FViewModelPropertyNodeHelper::AddInputViewModelPin(*this, ViewModelOwnerClass);
}

void UK2Node_ExplicitGetViewModelPropertyValue::ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const
{
    Super::ValidateNodeDuringCompilation(MessageLog);

    FViewModelPropertyNodeHelper::ValidateInputViewModelPin(this, MessageLog);
}

FText UK2Node_ExplicitGetViewModelPropertyValue::GetNodeTitleForCache(ENodeTitleType::Type TitleType) const
{
    return TitleType == ENodeTitleType::FullTitle
        ? FText::Format(NSLOCTEXT("UnrealMvvm", "ExplicitGetViewModelPropertyValue_TitleFull", "Get {0}\nFrom {1}"), FText::FromName(ViewModelPropertyName), GetViewModelDisplayName())
        : FText::Format(NSLOCTEXT("UnrealMvvm", "ExplicitGetViewModelPropertyValue_Title", "Get {0}"), FText::FromName(ViewModelPropertyName));
}
