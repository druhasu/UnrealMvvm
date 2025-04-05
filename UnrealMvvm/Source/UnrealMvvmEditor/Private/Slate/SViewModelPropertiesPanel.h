// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Input/Reply.h"
#include "Layout/Visibility.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"

class FBlueprintEditor;
class FViewModelPropertyBase;
class SViewModelPropertyRow;
class UK2Node_ViewModelPropertyChanged;
class FDetailColumnSizeData;

namespace UnrealMvvm_Impl
{
    struct FViewModelPropertyReflection;
}

class SViewModelPropertiesPanel : public SCompoundWidget
{
public:
    using SBindingListView = SListView<UK2Node_ViewModelPropertyChanged*>;
    using ThisClass = SViewModelPropertiesPanel;

    SLATE_BEGIN_ARGS(SViewModelPropertiesPanel) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, TSharedPtr<FBlueprintEditor> Editor);

private:
    TSharedRef<SWidget> MakeViewModelSelector();
    TSharedRef<SWidget> MakeAddBindingButton();
    TSharedRef<SWidget> MakeAddBindingPopup();
    void MakeAddBindingMenu(FMenuBuilder& Builder, UClass* ViewModelClass, TArray<FName> InPropertyPath);
    TSharedRef<SWidget> MakeContextMenuEntryWidget(const UnrealMvvm_Impl::FViewModelPropertyReflection& Reflection);

    TSharedRef<ITableRow> MakeBindingRow(UK2Node_ViewModelPropertyChanged* Node, const TSharedRef<STableViewBase>& OwnerTable);

    TSharedRef<SWidget> MakeViewModelClassSelector();

    void RegenerateBindings();
    void CacheViewModelClass(bool bMayRemoveExtension);
    void OnViewClassChanged(UClass* ViewClass, UClass* ViewModelClass);
    void OnBlueprintChanged(UBlueprint*);

    FText GetClassSelectorTooltip() const;
    EVisibility GetClassErrorVisibility() const;
    FText GetClassErrorTooltip() const;

    const UClass* GetViewModelClass() const;
    void OnViewModelClassSelected(const UClass* NewClass);

    void ShowViewModelSourceCode();
    FText GetGoToSourceTooltip() const;
    bool IsGoToSourceEnabled() const;

    void HandleAddBinding(TArray<FName> InPropertyPath);
    void HandleRemoveBinding(UK2Node_ViewModelPropertyChanged* Node);

    UK2Node_ViewModelPropertyChanged* FindEventNode(const TArray<FName>& InPropertyPath) const;

    TWeakPtr<FBlueprintEditor> WeakBlueprintEditor;
    TWeakObjectPtr<UBlueprint> Blueprint;

    bool bParentHasViewModel = false;
    UClass* ViewModelClass = nullptr;
    UClass* ParentViewModelClass = nullptr;

    TSharedPtr<SBindingListView> BindingList;
    TArray<UK2Node_ViewModelPropertyChanged*> BindingNodes;

    SHorizontalBox::FSlot* ClassSelectorSlot = nullptr;
};

class SViewModelPropertyRow : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SViewModelPropertyRow)
        : _NameContent()
        , _ValueContent()
        {}

        SLATE_NAMED_SLOT(FArguments, NameContent)
        SLATE_NAMED_SLOT(FArguments, ValueContent)

    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView, const TSharedPtr<FDetailColumnSizeData>& ColumnSizeData);

private:
    TSharedPtr<FDetailColumnSizeData> ColumnSizeData;
};
