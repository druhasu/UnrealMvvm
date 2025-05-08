// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "K2Node_CachedTexts.h"
#include "Mvvm/Impl/Property/ViewModelPropertyReflection.h"
#include "K2Node_ViewModelPropertyBase.generated.h"

/*
 * Base class for nodes that operate on a property of a ViewModel
 * Intended to be used inside WidgetBlueprint.
 */
UCLASS(Abstract)
class UNREALMVVMEDITOR_API UK2Node_ViewModelPropertyBase : public UK2Node_CachedTexts
{
    GENERATED_BODY()

public:
    //~ Begin UEdGraphNode Interface
    FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
    void ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const override;
    //~ End UEdGraphNode Interface

    //~ Begin UK2Node Interface
    bool IsActionFilteredOut(class FBlueprintActionFilter const& Filter) override;
    FText GetMenuCategory() const override;
    //~ End UK2Node Interface

    /* Class of ViewModel that this node is associated with */
    UPROPERTY()
    TObjectPtr<UClass> ViewModelOwnerClass;

    /* Property name that this node is associated with */
    UPROPERTY()
    FName ViewModelPropertyName;

protected:
    void AddSpawners(FBlueprintActionDatabaseRegistrar& ActionRegistrar, TFunctionRef<bool(const UnrealMvvm_Impl::FViewModelPropertyReflection&)> PropertyFilter) const;
    FText GetViewModelDisplayName() const { return ViewModelOwnerClass ? ViewModelOwnerClass->GetDisplayNameText() : FText::GetEmpty(); }
};
