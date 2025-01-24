// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "K2Node_CachedTexts.h"
#include "K2Node_ViewModelPropertyChanged.generated.h"

UCLASS()
class UK2Node_ViewModelPropertyChanged : public UK2Node_CachedTexts
{
    GENERATED_BODY()

public:
    //~ Begin UK2Node Interface
    void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
    bool NodeCausesStructuralBlueprintChange() const override { return true; }
    //~ End UK2Node Interface

    //~ Begin UEdGraphNode Interface
    void AllocateDefaultPins() override;
    void ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const override;
    FLinearColor GetNodeTitleColor() const override;
    FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
    //~ End UEdGraphNode Interface

    //~ Begin UObject Interface
    void Serialize(FArchive& Ar) override;
    //~ End UObject Interface

    /* Name of function to call, when this node binding is triggered */
    FName MakeCallbackName() const;

    /* Class of Viewmodel this node expects as root of property path */
    UClass* GetViewModelClass() const;

    /* Path of properties that this node listens */
    UPROPERTY()
    TArray<FName> PropertyPath;

protected:
    FText GetNodeTitleForCache(ENodeTitleType::Type TitleType) const override;
    FText GetTooltipTextForCache() const override;

private:
    static const FName IsInitialPinName;

    /* Legacy Property name that this node is associated with */
    UPROPERTY()
    FName ViewModelPropertyName_DEPRECATED;
};
