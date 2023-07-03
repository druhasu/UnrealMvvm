// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "K2Node.h"
#include "K2Node_ViewModelChanged.generated.h"

UCLASS()
class UK2Node_ViewModelChanged : public UK2Node
{
    GENERATED_BODY()

public:
    //~ Begin UK2Node Interface
    void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
    void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
    bool IsActionFilteredOut(class FBlueprintActionFilter const& Filter) override;
    //~ End UK2Node Interface

    //~ Begin UEdGraphNode Interface
    void PostReconstructNode() override;
    void AllocateDefaultPins() override;
    FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
    FLinearColor GetNodeTitleColor() const override;
    FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
    //~ End UEdGraphNode Interface

private:
    UClass* GetViewClass() const;
    void UpdatePinTypes();
    void OnViewClassChanged(UClass* ViewClass, UClass* ViewModelClass);

    static const FName OldViewModelPinName;
    static const FName NewViewModelPinName;
    static const FText NodeCategory;
};