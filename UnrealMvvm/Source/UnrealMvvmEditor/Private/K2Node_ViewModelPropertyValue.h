// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "K2Node.h"
#include "K2Node_ViewModelPropertyValue.generated.h"

/*
 * Custom node that returns value of a ViewModel property.
 * Intended to be used inside WidgetBlueprint.
 * This node registers custom Menu actions for all properties of a ViewModel
 */
UCLASS()
class UK2Node_ViewModelPropertyValue : public UK2Node
{
    GENERATED_BODY()

public:
    //~ Begin UObject Interface
    bool Modify(bool bAlwaysMarkDirty = true) override;
    //~ End UObject Interface

    //~ Begin UK2Node Interface
    bool IsNodePure() const override { return true; }
    void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
    void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
    //~ End UK2Node Interface

    //~ Begin UEdGraphNode Interface
    void AllocateDefaultPins() override;
    FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
    FLinearColor GetNodeTitleColor() const override;
    FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
    FText GetTooltipText() const override;
    //~ End UEdGraphNode Interface

    /* Class of ViewModel that this node is associated with */
    UPROPERTY()
    UClass* ViewModelOwnerClass;

    /* Property name that this node is associated with */
    UPROPERTY()
    FName ViewModelPropertyName;

private:
    /* Constructing FText strings can be costly, so we cache the node's title */
    FNodeTextCache CachedNodeTitle;
    FNodeTextCache CachedNodeTooltip;
};