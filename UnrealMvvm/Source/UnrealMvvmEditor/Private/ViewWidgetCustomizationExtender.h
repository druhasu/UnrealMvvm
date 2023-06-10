// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Customizations/IBlueprintWidgetCustomizationExtender.h"

class IDetailCategoryBuilder;

class FViewWidgetCustomizationExtender : public IBlueprintWidgetCustomizationExtender
{
    using ThisClass = FViewWidgetCustomizationExtender;

public:
    void Init();
    void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder, const TArrayView<UWidget*> InWidgets, const TSharedRef<FWidgetBlueprintEditor>& InWidgetBlueprintEditor) override;

private:
    void OnObjectPreSave(UObject* Asset, FObjectPreSaveContext Context);

    void CreateProperties(IDetailCategoryBuilder& Category, UClass* ViewModelClass, UBlueprint* Blueprint);

    const UClass* GetViewModelClass(UBlueprint* Blueprint) const;
    
    FText GetGoToSourceTooltip() const;
    bool GoToSourceEnabled(UBlueprint* Blueprint) const;

    FText GetClassSelectorTooltip(UBlueprint* Blueprint) const;
    bool ClassSelectorEnabled(UBlueprint* Blueprint) const;

    void HandleNavigateToViewModelClass(UBlueprint* Blueprint);
    void HandleSetClass(const UClass* NewClass, IDetailLayoutBuilder* DetailBuilder, UBlueprint* Blueprint);

    FText GetAddOrViewButtonTooltip(UClass* ViewModelClass, FName PropertyName, UBlueprint* Blueprint) const;
    FReply HandleAddOrViewEventForProperty(UClass* ViewModelClass, FName PropertyName, UBlueprint* Blueprint);
    int32 HandleAddOrViewIndexForButton(UClass* ViewModelClass, FName PropertyName, UBlueprint* Blueprint) const;

    class UK2Node_ViewModelPropertyChanged* FindEventNode(UClass* ViewModelClass, FName PropertyName, UBlueprint* Blueprint) const;

    static const FName CategoryName;
    static const FText CategoryNameText;
    static const FText ViewModelClassText;
};