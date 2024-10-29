// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "K2Node_ViewModelPropertyValueGetSet.h"
#include "K2Node_GetViewModelPropertyValue.generated.h"

/*
 * Custom node that returns value of a ViewModel property.
 * Intended to be used inside WidgetBlueprint or Actor blueprint.
 * This node registers custom Menu actions for all properties of a ViewModel
 */
UCLASS()
class UK2Node_GetViewModelPropertyValue : public UK2Node_ViewModelPropertyValueGetSet
{
    GENERATED_BODY()

public:
    //~ Begin UK2Node Interface
    bool IsNodePure() const override { return true; }
    void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
    void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
    //~ End UK2Node Interface

    //~ Begin UEdGraphNode Interface
    void AllocateDefaultPins() override;
    void ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const override;
    //~ End UEdGraphNode Interface

protected:
    FText GetNodeTitleForCache(ENodeTitleType::Type TitleType) const override;
    FText GetTooltipTextForCache() const override;
};
