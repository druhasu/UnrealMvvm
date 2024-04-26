// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "K2Node_SetViewModelPropertyValue.h"
#include "K2Node_ExplicitSetViewModelPropertyValue.generated.h"

/*
 * Custom node that sets value to a ViewModel property.
 * Unlike its parent node, explicitly accepts a ViewModel of required type as an input pin.
 * This node registers custom Menu actions for all properties of a ViewModel.
 */
UCLASS()
class UK2Node_ExplicitSetViewModelPropertyValue : public UK2Node_SetViewModelPropertyValue
{
    GENERATED_BODY()

public:
    //~ Begin UK2Node Interface
    void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
    FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
    bool IsActionFilteredOut(class FBlueprintActionFilter const& Filter) override;
    //~ End UK2Node Interface

    //~ Begin UEdGraphNode Interface
    void AllocateDefaultPins() override;
    void ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const;
    //~ End UEdGraphNode Interface

protected:
    FText GetNodeTitleForCache(ENodeTitleType::Type TitleType) const override;
};