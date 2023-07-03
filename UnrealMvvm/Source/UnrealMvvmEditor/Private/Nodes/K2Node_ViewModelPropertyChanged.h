// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "K2Node_ViewModelPropertyBase.h"
#include "Mvvm/Impl/ViewModelRegistry.h"
#include "K2Node_ViewModelPropertyChanged.generated.h"

UCLASS()
class UK2Node_ViewModelPropertyChanged : public UK2Node_ViewModelPropertyBase
{
    GENERATED_BODY()

public:
    //~ Begin UK2Node Interface
    void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
    //~ End UK2Node Interface

    //~ Begin UEdGraphNode Interface
    void AllocateDefaultPins() override;
    FLinearColor GetNodeTitleColor() const override;
    FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
    //~ End UEdGraphNode Interface

protected:
    FText GetNodeTitleForCache(ENodeTitleType::Type TitleType) const override;
};