// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "K2Node.h"
#include "Mvvm/Impl/ViewModelRegistry.h"
#include "K2Node_ViewModelPropertyChanged.generated.h"

UCLASS()
class UK2Node_ViewModelPropertyChanged : public UK2Node
{
    GENERATED_BODY()

public:
    //~ Begin UObject Interface
    bool Modify(bool bAlwaysMarkDirty = true) override;
    //~ End UObject Interface

    //~ Begin UK2Node Interface
    void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
    //~ End UK2Node Interface

    //~ Begin UEdGraphNode Interface
    void AllocateDefaultPins() override;
    FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
    FLinearColor GetNodeTitleColor() const override;
    FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
    //~ End UEdGraphNode Interface

    /* Class of ViewModel that this event is associated with */
    UPROPERTY()
    UClass* ViewModelOwnerClass;

    /* Property name that this event is associated with */
    UPROPERTY()
    FName ViewModelPropertyName;

private:
    /* Constructing FText strings can be costly, so we cache the node's title */
    FNodeTextCache CachedNodeTitle;
};