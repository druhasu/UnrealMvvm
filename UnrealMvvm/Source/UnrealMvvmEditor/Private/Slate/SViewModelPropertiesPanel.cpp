// Copyright Andrei Sudarikov. All Rights Reserved.

#include "SViewModelPropertiesPanel.h"
#include "Mvvm/BaseViewModel.h"
#include "Mvvm/ViewModelProperty.h"
#include "Mvvm/Impl/BaseView/ViewRegistry.h"
#include "Mvvm/Impl/Property/ViewModelPropertyIterator.h"
#include "BaseViewBlueprintExtension.h"
#include "Nodes/ViewModelPropertyNodeHelper.h"
#include "Nodes/K2Node_ViewModelPropertyChanged.h"

#include "EdGraphSchema_K2_Actions.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "PropertyCustomizationHelpers.h"
#include "BlueprintEditor.h"
#include "SourceCodeNavigation.h"
#include "DetailColumnSizeData.h"
#include "SViewModelBindingEntry.h"
#include "Widgets/Images/SLayeredImage.h"
#include "Styling/StyleColors.h"

void SViewModelPropertiesPanel::Construct(const FArguments& InArgs, TSharedPtr<FBlueprintEditor> Editor)
{
    WeakBlueprintEditor = Editor;
    Blueprint = Editor->GetBlueprintObj();

    Blueprint->OnChanged().AddSP(this, &ThisClass::OnBlueprintChanged);

    CacheViewModelClass(false); // false - so we are not marking Blueprint as modified right when it is opened
    RegenerateBindings();

    UnrealMvvm_Impl::FViewRegistry::ViewModelClassChanged.AddSP(this, &ThisClass::OnViewClassChanged);

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
        .AutoHeight()
        [
            MakeAddBindingButton()
        ]

        + SVerticalBox::Slot()
        [
            SNew(SOverlay)

            // Property Bindings list
            + SOverlay::Slot()
            .HAlign(HAlign_Fill)
            .VAlign(VAlign_Fill)
            [
                SAssignNew(BindingList, SBindingListView)
                .ListItemsSource(&BindingNodes)
                .SelectionMode(ESelectionMode::None)
                .OnGenerateRow(this, &ThisClass::MakeBindingRow)
                .ExternalScrollbar(ScrollBar)
            ]

            + SOverlay::Slot()
            .HAlign(HAlign_Right)
            .VAlign(VAlign_Fill)
            [
                SNew(SBorder)
                .Padding(0)
                .BorderImage(FAppStyle::Get().GetBrush("DetailsView.GridLine"))
                .Visibility(BindingList.ToSharedRef(), &STableViewBase::GetScrollbarVisibility)
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
    .Padding(8, 4, 20, 4)
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

TSharedRef<SWidget> SViewModelPropertiesPanel::MakeAddBindingButton()
{
    return SNew(SBox)
    .HAlign(HAlign_Left)
    .VAlign(VAlign_Center)
    .Padding(8, 0, 20, 4)
    [
        SNew(SComboButton)
        .OnGetMenuContent(this, &ThisClass::MakeAddBindingPopup)
        .ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("Button"))
        .HasDownArrow(false)
        .ContentPadding(FMargin(4, 2))
        .ButtonContent()
        [
            SNew(SHorizontalBox)

            + SHorizontalBox::Slot()
            .VAlign(VAlign_Center)
            .AutoWidth()
            .Padding(0, 0, 4, 0)
            [
                SNew(SImage)
                .ColorAndOpacity(FStyleColors::AccentGreen)
                .Image(FAppStyle::Get().GetBrush("Icons.Plus"))
            ]

            + SHorizontalBox::Slot()
            .VAlign(VAlign_Center)
            .AutoWidth()
            [
                SNew(STextBlock)
                .Text(NSLOCTEXT("UnrealMvvm", "AddViewModelBinding", "Add Binding"))
            ]
        ]
    ];
}

TSharedRef<SWidget> SViewModelPropertiesPanel::MakeAddBindingPopup()
{
    FMenuBuilder Builder(true, nullptr, nullptr, false, &FCoreStyle::Get(), true, NAME_None, false);

    MakeAddBindingMenu(Builder, ViewModelClass, {});

    return Builder.MakeWidget();
}

void SViewModelPropertiesPanel::MakeAddBindingMenu(FMenuBuilder& Builder, UClass* InViewModelClass, TArray<FName> InPropertyPath)
{
    using namespace UnrealMvvm_Impl;

    if (!InPropertyPath.IsEmpty())
    {
        // add entry for adding current property
        Builder.AddMenuEntry(
            INVTEXT("Bind"),
            {},
            FSlateIcon(),
            FUIAction(
                FExecuteAction::CreateSP(this, &ThisClass::HandleAddBinding, InPropertyPath)
            )
        );
    }

    Builder.BeginSection(NAME_None, InViewModelClass->GetDisplayNameText());

    auto CanBindToProperty = [](const FViewModelPropertyReflection* Reflection)
    {
        return FViewModelPropertyNodeHelper::IsPropertyAvailableInBlueprint(*Reflection) && Reflection->Flags.HasPublicGetter;
    };

    for (FViewModelPropertyIterator It(InViewModelClass, true); It; ++It)
    {
        TArray<FName> NewPropertyPath = InPropertyPath;
        NewPropertyPath.Add(It->GetProperty()->GetName());

        UClass* ValueClass = Cast<UClass>(It->GetPinSubCategoryObject());
        if (It->ContainerType == EPinContainerType::None && ValueClass != nullptr && ValueClass->IsChildOf<UBaseViewModel>())
        {
            // create sub menu
            Builder.AddSubMenu(
                MakeContextMenuEntryWidget(*It),
                FNewMenuDelegate::CreateSP(this, &ThisClass::MakeAddBindingMenu, ValueClass, NewPropertyPath)
            );
        }
        else
        {
            // create regular entry
            Builder.AddMenuEntry(
                FUIAction(
                    FExecuteAction::CreateSP(this, &ThisClass::HandleAddBinding, NewPropertyPath),
                    FCanExecuteAction::CreateLambda(CanBindToProperty, &*It)
                ),
                MakeContextMenuEntryWidget(*It)
            );
        }
    }

    Builder.EndSection();
}

TSharedRef<SWidget> SViewModelPropertiesPanel::MakeContextMenuEntryWidget(const UnrealMvvm_Impl::FViewModelPropertyReflection& Reflection)
{
    using namespace UnrealMvvm_Impl;

    FEdGraphPinType PinType;
    FViewModelPropertyNodeHelper::FillPinType(PinType, &Reflection);

    const UEdGraphSchema_K2* Schema = GetDefault<UEdGraphSchema_K2>();

    auto GetEntryTooltip = [Reflection]()
    {
        if (!FViewModelPropertyNodeHelper::IsPropertyAvailableInBlueprint(Reflection))
        {
            return NSLOCTEXT("UnrealMvvm", "PropertyNotAvailableInBP", "Not Available in Blueprints");
        }

        if (!Reflection.Flags.HasPublicGetter)
        {
            return NSLOCTEXT("UnrealMvvm", "PropertyHasNoPublicGetter", "Has no public Getter");
        }

        return FText::FromName(Reflection.GetProperty()->GetName());
    };

    TSharedRef<SHorizontalBox> HorizontalBox =
        SNew(SHorizontalBox)
        .ToolTipText_Lambda(GetEntryTooltip)

        + SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(0, 0, 4, 0)
        [
            SNew(SLayeredImage, FBlueprintEditorUtils::GetSecondaryIconFromPin(PinType), Schema->GetSecondaryPinTypeColor(PinType))
            .Image(FBlueprintEditorUtils::GetIconFromPin(PinType, /* bIsLarge = */ false))
            .ColorAndOpacity(Schema->GetPinTypeColor(PinType))
            .ToolTipText(Schema->TypeToText(PinType))
        ]

        + SHorizontalBox::Slot()
        .AutoWidth()
        [
            SNew(STextBlock)
            .Text(FText::FromName(Reflection.GetProperty()->GetName()))
            .OverflowPolicy(ETextOverflowPolicy::Ellipsis)
        ]
    ;

    return HorizontalBox;
}

TSharedRef<ITableRow> SViewModelPropertiesPanel::MakeBindingRow(UK2Node_ViewModelPropertyChanged* Node, const TSharedRef<STableViewBase>& OwnerTable)
{
    return SNew(STableRow<UK2Node_ViewModelPropertyChanged*>, OwnerTable)
    .Style(FAppStyle::Get(), "DetailsView.TreeView.TableRow")
    [
        SNew(SViewModelBindingEntry, OwnerTable, Node)
        .OnRemoveClicked(this, &ThisClass::HandleRemoveBinding, Node)
    ];
}

void SViewModelPropertiesPanel::RegenerateBindings()
{
    BindingNodes.Reset();
    FBlueprintEditorUtils::GetAllNodesOfClass<UK2Node_ViewModelPropertyChanged>(Blueprint.Get(), BindingNodes);
}

void SViewModelPropertiesPanel::CacheViewModelClass(bool bMayRemoveExtension)
{
    ViewModelClass = UnrealMvvm_Impl::FViewRegistry::GetViewModelClass(Blueprint->ParentClass);
    bParentHasViewModel = ViewModelClass != nullptr;

    if (!ViewModelClass)
    {
        if (UBaseViewBlueprintExtension* Extension = UBaseViewBlueprintExtension::Get(Blueprint.Get()))
        {
            ViewModelClass = Extension->GetViewModelClass();
        }
    }
    else if (bMayRemoveExtension && !Blueprint->ParentClass->IsNative())
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
        RegenerateBindings();
        BindingList->RebuildList();
    }
}

void SViewModelPropertiesPanel::OnBlueprintChanged(UBlueprint*)
{
    RegenerateBindings();
    BindingList->RebuildList();
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
    ViewModelClass = const_cast<UClass*>(NewClass);

    if (ViewModelClass)
    {
        UBaseViewBlueprintExtension* Extension = UBaseViewBlueprintExtension::Request(Blueprint.Get());
        Extension->SetViewModelClass(ViewModelClass);
    }
    else
    {
        UBaseViewBlueprintExtension::Remove(Blueprint.Get());
    }

    RegenerateBindings();
    BindingList->RebuildList();

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

void SViewModelPropertiesPanel::HandleAddBinding(TArray<FName> InPropertyPath)
{
    UEdGraph* TargetGraph = Blueprint->GetLastEditedUberGraph();

    if (TargetGraph != nullptr)
    {
        // Figure out a decent place to stick the node
        const FVector2D NewNodePos = TargetGraph->GetGoodPlaceForNewNode();

        UK2Node_ViewModelPropertyChanged* EventNode = FindEventNode(InPropertyPath);

        if (EventNode == nullptr)
        {
            // Create a new event node
            EventNode = FEdGraphSchemaAction_K2NewNode::SpawnNode<UK2Node_ViewModelPropertyChanged>(
                TargetGraph,
                NewNodePos,
                EK2NewNodeFlags::SelectNewNode,
                [this, PropertyPath = MoveTemp(InPropertyPath)](UK2Node_ViewModelPropertyChanged* NewInstance)
                {
                    NewInstance->PropertyPath = PropertyPath;
                }
            );
        }

        // Finally, bring up kismet and jump to the new node
        if (EventNode != nullptr)
        {
            FKismetEditorUtilities::BringKismetToFocusAttentionOnObject(EventNode);
        }
    }
}

void SViewModelPropertiesPanel::HandleRemoveBinding(UK2Node_ViewModelPropertyChanged* Node)
{
    const FScopedTransaction Transaction(INVTEXT("Remove binding"));

    FBlueprintEditorUtils::RemoveNode(Blueprint.Get(), Node);
}

UK2Node_ViewModelPropertyChanged* SViewModelPropertiesPanel::FindEventNode(const TArray<FName>& InPropertyPath) const
{
    TArray<UK2Node_ViewModelPropertyChanged*> EventNodes;
    FBlueprintEditorUtils::GetAllNodesOfClass(Blueprint.Get(), EventNodes);

    for (UK2Node_ViewModelPropertyChanged* BoundEvent : EventNodes)
    {
        if (BoundEvent->PropertyPath == InPropertyPath)
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
