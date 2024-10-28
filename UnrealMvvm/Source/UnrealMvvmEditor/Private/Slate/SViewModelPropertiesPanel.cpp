// Copyright Andrei Sudarikov. All Rights Reserved.

#include "SViewModelPropertiesPanel.h"
#include "Mvvm/BaseViewModel.h"
#include "Mvvm/ViewModelProperty.h"
#include "Mvvm/Impl/ViewModelPropertyIterator.h"
#include "BaseViewBlueprintExtension.h"
#include "Nodes/ViewModelPropertyNodeHelper.h"
#include "Nodes/K2Node_ViewModelPropertyChanged.h"
#include "BaseViewBlueprintExtension.h"

#include "Styling/StyleColors.h"
#include "DetailLayoutBuilder.h"
#include "EdGraphSchema_K2_Actions.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "PropertyCustomizationHelpers.h"
#include "BlueprintEditor.h"
#include "SourceCodeNavigation.h"
#include "DetailColumnSizeData.h"

#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Misc/EngineVersionComparison.h"

void SViewModelPropertiesPanel::Construct(const FArguments& InArgs, TSharedPtr<FBlueprintEditor> Editor)
{
    WeakBlueprintEditor = Editor;
    Blueprint = Editor->GetBlueprintObj();

    ColumnSizeData = MakeShared<FDetailColumnSizeData>();
    ColumnSizeData->SetValueColumnWidth(0.5f);

    CacheViewModelClass(false); // false - so we are not marking Blueprint as modified right when it is opened
    RegenerateProperties();

    UnrealMvvm_Impl::FViewModelRegistry::ViewClassChanged.AddSP(this, &ThisClass::OnViewClassChanged);

    TSharedRef<SScrollBar> ScrollBar = SNew(SScrollBar);

    ChildSlot
    [
        SNew(SVerticalBox)

        + SVerticalBox::Slot()
        .AutoHeight()
        [
            MakeViewModelSelector()
        ]

        + SVerticalBox::Slot()
        [
            SNew(SOverlay)

            + SOverlay::Slot()
            .HAlign(HAlign_Fill)
            .VAlign(VAlign_Fill)
            [
                // ViewModel properties list
                SAssignNew(PropertyList, SPropertyListView)
#if UE_VERSION_OLDER_THAN(5,5,0)
                .ItemHeight(26)
#endif
                .ListItemsSource(&Properties)
                .SelectionMode(ESelectionMode::None)
                .OnGenerateRow(this, &ThisClass::MakeViewModelPropertyRow)
                .ExternalScrollbar(ScrollBar)
            ]

            + SOverlay::Slot()
            .HAlign(HAlign_Right)
            .VAlign(VAlign_Fill)
            [
                SNew(SBorder)
                .Padding(0)
                .BorderImage(FAppStyle::Get().GetBrush("DetailsView.GridLine"))
                .Visibility(PropertyList.ToSharedRef(), &STableViewBase::GetScrollbarVisibility)
                [
                    SNew(SBox)
                    .WidthOverride(16)
                    [
                        ScrollBar
                    ]
                ]
            ]
        ]
    ];
}

TSharedRef<SWidget> SViewModelPropertiesPanel::MakeViewModelSelector()
{
    return SNew(SBox)
    .VAlign(VAlign_Center)
    .Padding(22, 4, 20, 4)
    .HeightOverride(34)
    [
        SNew(SHorizontalBox)
        
        + SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        .Padding(0, 0, 5, 0)
        [
            SNew(STextBlock)
            .Text(NSLOCTEXT("UnrealMvvm", "ViewModelClassText", "ViewModel Class"))
            .ToolTipText(this, &ThisClass::GetClassSelectorTooltip)
        ]

        // Add a class entry box.  Even though this isn't an class entry, we will simulate one
        + SHorizontalBox::Slot()
        .Padding(0, 0, 5, 0)
        [
            SNew(SClassPropertyEntryBox)
            .MetaClass(UBaseViewModel::StaticClass())
            .AllowAbstract(true)
            .AllowNone(true)
            .ShowTreeView(false)
            .HideViewOptions(false)
            .ShowDisplayNames(true)
            .SelectedClass(this, &ThisClass::GetViewModelClass)
            .OnSetClass(this, &ThisClass::OnViewModelClassSelected)
            .IsEnabled(this, &ThisClass::IsClassSelectorEnabled)
        ]

        // Add "Go to Source" button
        + SHorizontalBox::Slot()
        .AutoWidth()
        [
            PropertyCustomizationHelpers::MakeBrowseButton(
                FSimpleDelegate::CreateSP(this, &ThisClass::ShowViewModelSourceCode),
                GetGoToSourceTooltip(),
                TAttribute<bool>::CreateSP(this, &ThisClass::IsGoToSourceEnabled)
            )
        ]
    ];
}

TSharedRef<ITableRow> SViewModelPropertiesPanel::MakeViewModelPropertyRow(TSharedRef<FListItem> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
    FName PropertyName = Item->Reflection->GetProperty()->GetName();

    return SNew(STableRow<TSharedRef<const UnrealMvvm_Impl::FViewModelPropertyReflection*>>, OwnerTable)
    .Style(FAppStyle::Get(), "DetailsView.TreeView.TableRow")
    [
        SNew(SViewModelPropertyRow, OwnerTable, ColumnSizeData)
        .NameContent()
        [
            MakePropertyNameContent(FText::FromName(PropertyName))
        ]
        .ValueContent()
        [
            MakePropertyValueContent(Item->Reflection, PropertyName)
        ]
    ];
}

TSharedRef<SWidget> SViewModelPropertiesPanel::MakePropertyNameContent(const FText& NameText)
{
    return SNew(SHorizontalBox)

    + SHorizontalBox::Slot()
    .AutoWidth()
    .VAlign(VAlign_Center)
    .Padding(0, 0, 5, 0)
    [
        SNew(SImage)
        .Image(FAppStyle::GetBrush("GraphEditor.Event_16x"))
    ]

    + SHorizontalBox::Slot()
    .VAlign(VAlign_Center)
    [
        SNew(STextBlock)
        .Font(IDetailLayoutBuilder::GetDetailFont())
        .Text(NameText)
        .ToolTipText(NameText)
    ];
}

TSharedRef<SWidget> SViewModelPropertiesPanel::MakePropertyValueContent(const UnrealMvvm_Impl::FViewModelPropertyReflection* Item, FName PropertyName)
{
    TSharedPtr<SWidget> Widget;

    if (FViewModelPropertyNodeHelper::IsPropertyAvailableInBlueprint(*Item))
    {
        if(Item->Flags.HasPublicGetter)
        {
            Widget = SNew(SButton)
            .HAlign(HAlign_Center)
            .ContentPadding(FMargin(20.0, 2.0))
            .ToolTipText(this, &ThisClass::GetAddOrViewButtonTooltip, PropertyName)
            .IsEnabled(this, &ThisClass::IsAddOrViewButtonEnabled, PropertyName)
            .OnClicked(this, &ThisClass::HandleAddOrViewEventForProperty, PropertyName)
            [
                SNew(SBox)
                .MinDesiredHeight(14)
                .VAlign(VAlign_Center)
                [
                    SNew(SWidgetSwitcher)
                    .WidgetIndex(this, &ThisClass::HandleAddOrViewIndexForButton, PropertyName)
                    + SWidgetSwitcher::Slot()
                    [
                        SNew(SImage)
                        .ColorAndOpacity(FSlateColor::UseForeground())
                        .Image(FAppStyle::Get().GetBrush("Icons.SelectInViewport"))
                    ]
                    + SWidgetSwitcher::Slot()
                    [
                        SNew(SImage)
                        .ColorAndOpacity(FSlateColor::UseForeground())
                        .Image(FAppStyle::Get().GetBrush("Icons.Plus"))
                    ]
                ]
            ];
        }
        else
        {
            Widget = SNew(STextBlock)
            .Font(IDetailLayoutBuilder::GetDetailFont())
            .Text(NSLOCTEXT("UnrealMvvm", "PropertyHasNoPublicGetter", "Has no public Getter"));
        }
    }
    else
    {
        Widget = SNew(STextBlock)
        .Font(IDetailLayoutBuilder::GetDetailFont())
        .Text(NSLOCTEXT("UnrealMvvm", "PropertyNotAvailableInBP", "Not Available in Blueprints"));
    }

    return SNew(SBox)
    .MinDesiredHeight(22)
    .VAlign(VAlign_Center)
    .HAlign(HAlign_Left)
    [
        Widget.ToSharedRef()
    ];
}

void SViewModelPropertiesPanel::RegenerateProperties()
{
    Properties.Reset();

    if (ViewModelClass)
    {
        UnrealMvvm_Impl::FViewModelPropertyIterator Iter(const_cast<UClass*>(ViewModelClass), true);

        // Add items for each property in ViewModel
        for (; Iter; ++Iter)
        {
            Properties.Emplace(MakeShared<FListItem>(FListItem{ &*Iter }));
        }
    }
}

void SViewModelPropertiesPanel::CacheViewModelClass(bool bMayRemoveExtension)
{
    ViewModelClass = UnrealMvvm_Impl::FViewModelRegistry::GetViewModelClass(Blueprint->ParentClass);
    bParentHasViewModel = ViewModelClass != nullptr;

    if (!ViewModelClass)
    {
        if (UBaseViewBlueprintExtension* Extension = UBaseViewBlueprintExtension::Get(Blueprint.Get()))
        {
            ViewModelClass = Extension->GetViewModelClass();
        }
    }
    else if(bMayRemoveExtension)
    {
        // our parent class defines ViewModel, no need to keep our own extension
        UBaseViewBlueprintExtension::Remove(Blueprint.Get());

        FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint.Get());
    }
}

void SViewModelPropertiesPanel::OnViewClassChanged(UClass* ViewClass, UClass* InViewModelClass)
{
    if (Blueprint->ParentClass->IsChildOf(ViewClass))
    {
        CacheViewModelClass(true);
        RegenerateProperties();
        PropertyList->RebuildList();
    }
}

FText SViewModelPropertiesPanel::GetClassSelectorTooltip() const
{
    return IsClassSelectorEnabled() ?
        NSLOCTEXT("UnrealMvvm", "ClassSelectorEnabledTooltip", "Select ViewModel class for this View") :
        NSLOCTEXT("UnrealMvvm", "ClassSelectorDisaabledTooltip", "ViewModel class is set in Parent class and cannot be changed in this Blueprint");
}

bool SViewModelPropertiesPanel::IsClassSelectorEnabled() const
{
    return !bParentHasViewModel;
}

const UClass* SViewModelPropertiesPanel::GetViewModelClass() const
{
    return ViewModelClass;
}

void SViewModelPropertiesPanel::OnViewModelClassSelected(const UClass* NewClass)
{
    ViewModelClass = NewClass;

    if (ViewModelClass)
    {
        UBaseViewBlueprintExtension* Extension = UBaseViewBlueprintExtension::Request(Blueprint.Get());
        Extension->SetViewModelClass(const_cast<UClass*>(ViewModelClass));
    }
    else
    {
        UBaseViewBlueprintExtension::Remove(Blueprint.Get());
    }

    RegenerateProperties();
    PropertyList->RebuildList();

    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint.Get());
}

void SViewModelPropertiesPanel::ShowViewModelSourceCode()
{
    if (ensure(ViewModelClass))
    {
        FSourceCodeNavigation::NavigateToClass(ViewModelClass);
    }
}

FText SViewModelPropertiesPanel::GetGoToSourceTooltip() const
{
    return FText::Format(NSLOCTEXT("UnrealMvvm", "GotoViewModelSourceTooltip", "Click to open ViewModel source file in {0}"), FSourceCodeNavigation::GetSelectedSourceCodeIDE());
}

bool SViewModelPropertiesPanel::IsGoToSourceEnabled() const
{
    return ViewModelClass != nullptr;
}

FText SViewModelPropertiesPanel::GetAddOrViewButtonTooltip(FName PropertyName) const
{
    if (!IsAddOrViewButtonEnabled(PropertyName))
    {
        return NSLOCTEXT("UnrealMvvm", "DisabledEventTooltip", "Property changed handler already exist in parent class.\nMultiple handlers are not supported yet");
    }

    UK2Node_ViewModelPropertyChanged* EventNode = FindEventNode(PropertyName);

    return EventNode ?
        NSLOCTEXT("UnrealMvvm", "ViewEventTooltip", "View property changed handler") :
        NSLOCTEXT("UnrealMvvm", "AddEventTooltip", "Add property changed handler");
}

bool SViewModelPropertiesPanel::IsAddOrViewButtonEnabled(FName PropertyName) const
{
    auto ReflectionInfo = UnrealMvvm_Impl::FViewModelRegistry::FindProperty(const_cast<UClass*>(ViewModelClass), PropertyName);

    if (ReflectionInfo)
    {
        UFunction* Function = Blueprint->ParentClass->FindFunctionByName(ReflectionInfo->GetProperty()->GetCallbackName());
        return Function == nullptr;
    }

    return true;
}

FReply SViewModelPropertiesPanel::HandleAddOrViewEventForProperty(FName PropertyName)
{
    UEdGraph* TargetGraph = Blueprint->GetLastEditedUberGraph();

    if (TargetGraph != nullptr)
    {
        UK2Node_ViewModelPropertyChanged* EventNode = FindEventNode(PropertyName);

        if (EventNode == nullptr)
        {
            // Figure out a decent place to stick the node
            const FVector2D NewNodePos = TargetGraph->GetGoodPlaceForNewNode();

            // Create a new event node
            EventNode = FEdGraphSchemaAction_K2NewNode::SpawnNode<UK2Node_ViewModelPropertyChanged>(
                TargetGraph,
                NewNodePos,
                EK2NewNodeFlags::SelectNewNode,
                [this, PropertyName](UK2Node_ViewModelPropertyChanged* NewInstance)
                {
                    NewInstance->ViewModelOwnerClass = const_cast<UClass*>(ViewModelClass);
                    NewInstance->ViewModelPropertyName = PropertyName;
                }
            );
        }

        // Finally, bring up kismet and jump to the new node
        if (EventNode != nullptr)
        {
            FKismetEditorUtilities::BringKismetToFocusAttentionOnObject(EventNode);
        }
    }

    return FReply::Handled();
}

int32 SViewModelPropertiesPanel::HandleAddOrViewIndexForButton(FName PropertyName) const
{
    UK2Node_ViewModelPropertyChanged* EventNode = FindEventNode(PropertyName);

    return EventNode ? 0 : 1;
}

UK2Node_ViewModelPropertyChanged* SViewModelPropertiesPanel::FindEventNode(FName PropertyName) const
{
    TArray<UK2Node_ViewModelPropertyChanged*> EventNodes;
    FBlueprintEditorUtils::GetAllNodesOfClass(Blueprint.Get(), EventNodes);

    for (UK2Node_ViewModelPropertyChanged* BoundEvent : EventNodes)
    {
        if (BoundEvent->ViewModelPropertyName == PropertyName &&
            BoundEvent->ViewModelOwnerClass == ViewModelClass)
        {
            return BoundEvent;
        }
    }

    return nullptr;
}

void SViewModelPropertyRow::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView, const TSharedPtr<FDetailColumnSizeData>& InColumnSizeData)
{
    ColumnSizeData = InColumnSizeData;

    const FMargin LeftRowPadding(22.0f, 0.0f, 10.0f, 0.0f);
    const FMargin RightRowPadding(12.0f, 0.0f, 2.0f, 0.0f);

    TSharedRef<SSplitter> Splitter = SNew(SSplitter)
    .Style(FAppStyle::Get(), "DetailsView.Splitter")
    .PhysicalSplitterHandleSize(1.0f)
    .HitDetectionSplitterHandleSize(5.0f)
    .HighlightedHandleIndex(ColumnSizeData->GetHoveredSplitterIndex())
    .OnHandleHovered(ColumnSizeData->GetOnSplitterHandleHovered())

    + SSplitter::Slot()
    .Value(ColumnSizeData->GetNameColumnWidth())
    .OnSlotResized(ColumnSizeData->GetOnNameColumnResized())
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .VAlign(VAlign_Center)
        .Padding(LeftRowPadding)
        [
            InArgs._NameContent.Widget
        ]
    ]

    + SSplitter::Slot()
    .Value(ColumnSizeData->GetValueColumnWidth())
    .OnSlotResized(ColumnSizeData->GetOnValueColumnResized())
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .VAlign(VAlign_Center)
        .Padding(RightRowPadding)
        [
            InArgs._ValueContent.Widget
        ]
    ];

    auto GetBorderBackground = [this]()
    {
        return IsHovered() ?
            FAppStyle::Get().GetSlateColor("Colors.Header") :
            FAppStyle::Get().GetSlateColor("Colors.Panel");
    };

    ChildSlot[
        SNew( SBorder )
        .BorderImage(FAppStyle::Get().GetBrush("DetailsView.GridLine"))
        .Padding(FMargin(0,0,0,1))
        .Clipping(EWidgetClipping::ClipToBounds)
        [
            SNew(SBox)
            .MinDesiredHeight(26)
            [
                SNew( SBorder )
                .BorderImage(FAppStyle::Get().GetBrush("DetailsView.CategoryMiddle"))
                .BorderBackgroundColor_Lambda(GetBorderBackground)
                .Padding(FMargin(0, 0, 16, 0))
                [
                    Splitter
                ]
            ]
        ]
    ];
}
