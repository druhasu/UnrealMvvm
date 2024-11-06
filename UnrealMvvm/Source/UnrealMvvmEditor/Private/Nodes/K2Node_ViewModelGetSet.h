// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "K2Node.h"
#include "K2Node_ViewModelGetSet.generated.h"

UCLASS()
class UK2Node_ViewModelGetSet : public UK2Node
{
    GENERATED_BODY()

public:
    //~ Begin UK2Node Interface
    bool IsNodePure() const override { return !bIsSetter; }
    void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
    void NotifyPinConnectionListChanged(UEdGraphPin* Pin) override;
    void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
    //~ End UK2Node Interface

    //~ Begin UEdGraphNode Interface
    void PostReconstructNode() override;
    void AllocateDefaultPins() override;
    FString GetPinMetaData(FName InPinName, FName InKey) override;
    FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
    FLinearColor GetNodeTitleColor() const override;
    FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
    FText GetTooltipText() const override;
    //~ End UEdGraphNode Interface

private:
    void UpdateViewModelPinType();
    UClass* GetViewClass() const;
    FName GetFunctionName() const;
    void OnViewClassChanged(UClass* ViewClass, UClass* ViewModelClass);

    static const FText GetViewModelNameText;
    static const FText SetViewModelNameText;
    static const FText NodeCategory;

    UPROPERTY()
    bool bIsSetter = true;
};
