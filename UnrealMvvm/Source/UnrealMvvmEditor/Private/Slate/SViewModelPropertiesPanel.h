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
    struct FListItem
    {
        const UnrealMvvm_Impl::FViewModelPropertyReflection* Reflection;
    };

    using SPropertyListView = SListView<TSharedRef<FListItem>>;
    using ThisClass = SViewModelPropertiesPanel;

    SLATE_BEGIN_ARGS(SViewModelPropertiesPanel) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, TSharedPtr<FBlueprintEditor> Editor);

private:
    TSharedRef<SWidget> MakeViewModelSelector();
    TSharedRef<ITableRow> MakeViewModelPropertyRow(TSharedRef<FListItem> Item, const TSharedRef<STableViewBase>& OwnerTable);

    TSharedRef<SWidget> MakePropertyNameContent(const FText& NameText);
    TSharedRef<SWidget> MakePropertyValueContent(const UnrealMvvm_Impl::FViewModelPropertyReflection* Item, FName PropertyName);

    void RegenerateProperties();
    void CacheViewModelClass();

    FText GetClassSelectorTooltip() const;
    bool IsClassSelectorEnabled() const;

    const UClass* GetViewModelClass() const;
    void OnViewModelClassSelected(const UClass* NewClass);

    void ShowViewModelSourceCode();
    FText GetGoToSourceTooltip() const;
    bool IsGoToSourceEnabled() const;

    FText GetAddOrViewButtonTooltip(FName PropertyName) const;
    bool IsAddOrViewButtonEnabled(FName PropertyName) const;

    FReply HandleAddOrViewEventForProperty(FName PropertyName);
    int32 HandleAddOrViewIndexForButton(FName PropertyName) const;

    UK2Node_ViewModelPropertyChanged* FindEventNode(FName PropertyName) const;

    TWeakPtr<FBlueprintEditor> WeakBlueprintEditor;
    TWeakObjectPtr<UBlueprint> Blueprint;

    bool bParentHasViewModel = false;
    const UClass* ViewModelClass = nullptr;

    TSharedPtr<SPropertyListView> PropertyList;
    TArray<TSharedRef<FListItem>> Properties;

    /** Container used by all splitters in the details view, so that they move in sync */
    TSharedPtr<FDetailColumnSizeData> ColumnSizeData;
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