// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "K2Node.h"
#include "Mvvm/Impl/ViewModelPropertyReflection.h"
#include "EdGraph/EdGraphNodeUtils.h"
#include "K2Node_ViewModelPropertyBase.generated.h"

/*
 * Base class for nodes that operate on a property of a ViewModel
 * Intended to be used inside WidgetBlueprint.
 */
UCLASS(Abstract)
class UNREALMVVMEDITOR_API UK2Node_ViewModelPropertyBase : public UK2Node
{
    GENERATED_BODY()

public:
    //~ Begin UObject Interface
    bool Modify(bool bAlwaysMarkDirty = true) override;
    //~ End UObject Interface

    //~ Begin UEdGraphNode Interface
    FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
    FText GetTooltipText() const override;
    FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
    void ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const override;
    //~ End UEdGraphNode Interface

    //~ Begin UK2Node Interface
    bool IsActionFilteredOut(class FBlueprintActionFilter const& Filter) override;
    FText GetMenuCategory() const override;
    //~ End UK2Node Interface

    /* Class of ViewModel that this node is associated with */
    UPROPERTY()
    UClass* ViewModelOwnerClass;

    /* Property name that this node is associated with */
    UPROPERTY()
    FName ViewModelPropertyName;

protected:
    virtual FText GetNodeTitleForCache(ENodeTitleType::Type TitleType) const { return FText::GetEmpty(); };
    virtual FText GetTooltipTextForCache() const { return FText::GetEmpty(); };

    void AddSpawners(FBlueprintActionDatabaseRegistrar& ActionRegistrar, TFunctionRef<bool(const UnrealMvvm_Impl::FViewModelPropertyReflection&)> PropertyFilter) const;
    FText GetViewModelDisplayName() const { return ViewModelOwnerClass ? ViewModelOwnerClass->GetDisplayNameText() : FText::GetEmpty(); }

private:
    /* Constructing FText strings can be costly, so we cache the node's title */
    FNodeTextTable CachedTexts;
};