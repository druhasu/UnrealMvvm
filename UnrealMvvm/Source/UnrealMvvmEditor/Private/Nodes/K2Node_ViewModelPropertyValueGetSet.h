// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "K2Node_ViewModelPropertyBase.h"
#include "K2Node_ViewModelPropertyValueGetSet.generated.h"

/*
 * Base node class for UK2Node_GetViewModelPropertyValue and UK2Node_SetViewModelPropertyValue
 */
UCLASS()
class UK2Node_ViewModelPropertyValueGetSet : public UK2Node_ViewModelPropertyBase
{
    GENERATED_BODY()

public:
    //~ Begin UK2Node Interface
    void PostPlacedNewNode() override;
    void NotifyPinConnectionListChanged(UEdGraphPin* Pin) override;
    void ReconstructNode() override;
    //~ End UK2Node Interface

    //~ Begin UEdGraphNode Interface
    void AllocateDefaultPins() override;
    void ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const override;
    void GetNodeContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const override;
    bool IncludeParentNodeContextMenu() const override { return true; }
    //~ End UEdGraphNode Interface

    /* Whether to show ViewModel pin on a node */
    UPROPERTY()
    bool bShowViewModelPin = false;

protected:
    void UpdateViewModelPinVisibility(UEdGraphPin* Pin);
    void ToggleViewModelPin();
};
