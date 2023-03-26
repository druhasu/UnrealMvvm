// Copyright Andrei Sudarikov. All Rights Reserved.

#include "ViewWidgetCustomizationExtender.h"

#include "Components/Widget.h"
#include "UObject/UObjectGlobals.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Layout/SWidgetSwitcher.h"

#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "WidgetBlueprintEditor.h"
#include "EdGraphSchema_K2_Actions.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "PropertyCustomizationHelpers.h"
#include "Styling/AppStyle.h"
#include "SourceCodeNavigation.h"

#include "Mvvm/Impl/ViewModelRegistry.h"
#include "Mvvm/BaseViewModel.h"
#include "Mvvm/BaseView.h"
#include "Mvvm/Impl/BaseViewClassExtension.h"
#include "Mvvm/Impl/ViewModelPropertyIterator.h"

#include "BaseViewBlueprintExtension.h"
#include "ViewModelPropertyNodeHelper.h"
#include "K2Node_ViewModelPropertyChanged.h"

const FName FViewWidgetCustomizationExtender::CategoryName = "Base View Settings";
const FText FViewWidgetCustomizationExtender::CategoryNameText = NSLOCTEXT("UnrealMvvm", "CategoryNameText", "ViewModel Properties");
const FText FViewWidgetCustomizationExtender::ViewModelClassText = NSLOCTEXT("UnrealMvvm", "ViewModelClassText", "ViewModel Class");

void FViewWidgetCustomizationExtender::Init()
{
    // we have to use this approach, because we cannot call AddSP delegates from constructor
    FCoreUObjectDelegates::OnObjectPreSave.AddSP(this, &FViewWidgetCustomizationExtender::OnObjectPreSave);
}

void FViewWidgetCustomizationExtender::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder, const TArrayView<UWidget*> InWidgets, const TSharedRef<FWidgetBlueprintEditor>& InWidgetBlueprintEditor)
{
    using namespace UnrealMvvm_Impl;

    UBlueprint* Blueprint = nullptr;
    for (UObject* EditedAsset : *InWidgetBlueprintEditor->GetObjectsCurrentlyBeingEdited())
    {
        UBlueprint* EditedBlueprint = Cast<UBlueprint>(EditedAsset);
        if (EditedBlueprint && EditedBlueprint->GeneratedClass == InWidgets[0]->GetClass())
        {
            Blueprint = EditedBlueprint;
        }
    }
    
    if (InWidgets.Num() != 1 || Blueprint == nullptr)
    {
        return;
    }

    IDetailCategoryBuilder& Category = DetailBuilder.EditCategory(CategoryName, CategoryNameText, ECategoryPriority::TypeSpecific);

    Category.AddCustomRow(ViewModelClassText)
    .NameContent()
    [
        SNew(STextBlock)
        .Font(IDetailLayoutBuilder::GetDetailFont())
        .Text(ViewModelClassText)
        .ToolTipText(this, &FViewWidgetCustomizationExtender::GetClassSelectorTooltip, Blueprint)
    ]
    .ValueContent()
    .MinDesiredWidth(0)
    .MaxDesiredWidth(300)
    [
        SNew(SHorizontalBox)

        // Add a class entry box.  Even though this isn't an class entry, we will simulate one
        + SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        .Padding(0, 0, 5, 0)
        [
            SNew(SClassPropertyEntryBox)
            .MetaClass(UBaseViewModel::StaticClass())
            .AllowAbstract(true)
            .AllowNone(true)
            .ShowTreeView(false)
            .HideViewOptions(false)
            .ShowDisplayNames(true)
            .SelectedClass(this, &FViewWidgetCustomizationExtender::GetViewModelClass, Blueprint)
            .OnSetClass(this, &FViewWidgetCustomizationExtender::HandleSetClass, &Category.GetParentLayout(), Blueprint)
            .IsEnabled(this, &FViewWidgetCustomizationExtender::ClassSelectorEnabled, Blueprint)
        ]

        // Add "Go to Source" button
        + SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        .Padding(0, 0, 5, 0)
        [
            PropertyCustomizationHelpers::MakeBrowseButton(
                FSimpleDelegate::CreateSP(this, &FViewWidgetCustomizationExtender::HandleNavigateToViewModelClass, Blueprint),
                GetGoToSourceTooltip(),
                TAttribute<bool>::CreateSP(this, &FViewWidgetCustomizationExtender::GoToSourceEnabled, Blueprint)
            )
        ]
    ];

    const UClass* ViewModelClass = GetViewModelClass(Blueprint);
    if (ViewModelClass)
    {
        CreateProperties(Category, const_cast<UClass*>(ViewModelClass), Blueprint);
    }
}

void FViewWidgetCustomizationExtender::OnObjectPreSave(UObject* Asset, FObjectPreSaveContext Context)
{
    using namespace UnrealMvvm_Impl;

    UWidgetBlueprint* SavedBlueprint = Cast<UWidgetBlueprint>(Asset);
    if (!SavedBlueprint)
    {
        // it's not a widget, ignore
        return;
    }

    UClass* NativeViewClass = FBlueprintEditorUtils::GetNativeParent(SavedBlueprint);
    UClass* ViewModelClass = FViewModelRegistry::GetViewModelClass(NativeViewClass);

    if (NativeViewClass == nullptr)
    {
        // this widget was saved with base class that no longer exist, ignore it
        return;
    }

    if (NativeViewClass->IsChildOf<UBaseView>() || ViewModelClass == nullptr)
    {
        // it's either BaseView (that handles everything itself) or not a View at all, ignore
        return;
    }

    // save ViewModel class inside the asset
    UBaseViewBlueprintExtension* Extension = UWidgetBlueprintExtension::RequestExtension<UBaseViewBlueprintExtension>(Cast<UWidgetBlueprint>(SavedBlueprint));
    Extension->ViewModelClass = ViewModelClass;
}

void FViewWidgetCustomizationExtender::CreateProperties(IDetailCategoryBuilder& Category, UClass* ViewModelClass, UBlueprint* Blueprint)
{
    UnrealMvvm_Impl::FViewModelPropertyIterator Iter(ViewModelClass, true);

    // Add rows for each property in ViewModel
    for (; Iter; ++Iter)
    {
        FName PropertyName = Iter->GetProperty()->GetName();
        FText NameText = FText::FromName(PropertyName);
        FDetailWidgetRow& DetailRow = Category.AddCustomRow(NameText);

        DetailRow
        .NameContent()
        [
            SNew(SHorizontalBox)

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
            ]
        ];

        if (FViewModelPropertyNodeHelper::IsPropertyAvailableInBlueprint(*Iter))
        {
            DetailRow
            .ValueContent()
            .MinDesiredWidth(150)
            .MaxDesiredWidth(200)
            [
                SNew(SButton)
                .ButtonStyle(FAppStyle::Get(), "FlatButton.Success")
                .HAlign(HAlign_Center)
                .OnClicked(this, &FViewWidgetCustomizationExtender::HandleAddOrViewEventForProperty, ViewModelClass, PropertyName, Blueprint)
                .ForegroundColor(FSlateColor::UseForeground())
                [
                    SNew(SBox)
                    .MinDesiredHeight(14)
                    .VAlign(VAlign_Center)
                    [
                        SNew(SWidgetSwitcher)
                        .WidgetIndex(this, &FViewWidgetCustomizationExtender::HandleAddOrViewIndexForButton, ViewModelClass, PropertyName, Blueprint)
                        + SWidgetSwitcher::Slot()
                        [
                            SNew(STextBlock)
                            .Font(FAppStyle::GetFontStyle(TEXT("BoldFont")))
                            .Text(NSLOCTEXT("UnrealMvvm", "ViewEvent", "View"))
                        ]
                        + SWidgetSwitcher::Slot()
                        [
                            SNew(SImage)
                            .Image(FAppStyle::GetBrush("Plus"))
                        ]
                    ]
                ]
            ];
        }
        else
        {
            DetailRow
            .ValueContent()
            [
                SNew(SBox)
                .MinDesiredHeight(22)
                .VAlign(VAlign_Center)
                [
                    SNew(STextBlock)
                    .Font(IDetailLayoutBuilder::GetDetailFont())
                    .Text(NSLOCTEXT("UnrealMvvm", "PropertyNotAvailableInBP", "Not Available in Blueprints"))
                ]
            ];
        }
    }
}

const UClass* FViewWidgetCustomizationExtender::GetViewModelClass(UBlueprint* Blueprint) const
{
    UClass* Result = UnrealMvvm_Impl::FViewModelRegistry::GetViewModelClass(FBlueprintEditorUtils::GetNativeParent(Blueprint));
    
    if (!Result)
    {
        UBaseViewBlueprintExtension* Extension = UWidgetBlueprintExtension::GetExtension<UBaseViewBlueprintExtension>(Cast<UWidgetBlueprint>(Blueprint));

        Result = Extension ? Extension->ViewModelClass : nullptr;
    }

    return Result;
}

FText FViewWidgetCustomizationExtender::GetGoToSourceTooltip() const
{
    return FText::Format(NSLOCTEXT("UnrealMvvm", "GotoViewModelSourceTooltip", "Click to open ViewModel source file in {0}"), FSourceCodeNavigation::GetSelectedSourceCodeIDE());
}

bool FViewWidgetCustomizationExtender::GoToSourceEnabled(UBlueprint* Blueprint) const
{
    return GetViewModelClass(Blueprint) != nullptr;
}

FText FViewWidgetCustomizationExtender::GetClassSelectorTooltip(UBlueprint* Blueprint) const
{
    return ClassSelectorEnabled(Blueprint) ?
        NSLOCTEXT("UnrealMvvm", "ClassSelectorEnabledTooltip", "Select ViewModel class for this View") :
        NSLOCTEXT("UnrealMvvm", "ClassSelectorDisaabledTooltip", "ViewModel class is defined in code and cannot be changed in Blueprint");
}

bool FViewWidgetCustomizationExtender::ClassSelectorEnabled(UBlueprint* Blueprint) const
{
    return UnrealMvvm_Impl::FViewModelRegistry::GetViewModelClass(FBlueprintEditorUtils::GetNativeParent(Blueprint)) == nullptr;
}

void FViewWidgetCustomizationExtender::HandleNavigateToViewModelClass(UBlueprint* Blueprint)
{
    const UClass* ViewModelClass = GetViewModelClass(Blueprint);
    if (ensure(ViewModelClass))
    {
        FSourceCodeNavigation::NavigateToClass(ViewModelClass);
    }
}

void FViewWidgetCustomizationExtender::HandleSetClass(const UClass* NewClass, IDetailLayoutBuilder* DetailBuilder, UBlueprint* Blueprint)
{
    if (NewClass)
    {
        UBaseViewBlueprintExtension* Extension = UWidgetBlueprintExtension::RequestExtension<UBaseViewBlueprintExtension>(Cast<UWidgetBlueprint>(Blueprint));
        Extension->ViewModelClass = const_cast<UClass*>(NewClass);
    }
    else
    {
        UBaseViewBlueprintExtension* Extension = UWidgetBlueprintExtension::GetExtension<UBaseViewBlueprintExtension>(Cast<UWidgetBlueprint>(Blueprint));

        if (Extension)
        {
            Blueprint->RemoveExtension(Extension);
        }
    }

    DetailBuilder->ForceRefreshDetails();
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
}

FReply FViewWidgetCustomizationExtender::HandleAddOrViewEventForProperty(UClass* ViewModelClass, FName PropertyName, UBlueprint* Blueprint)
{
    UEdGraph* TargetGraph = Blueprint->GetLastEditedUberGraph();

    if (TargetGraph != nullptr)
    {
        UK2Node_ViewModelPropertyChanged* EventNode = FindEventNode(ViewModelClass, PropertyName, Blueprint);

        if (EventNode == nullptr)
        {
            // Figure out a decent place to stick the node
            const FVector2D NewNodePos = TargetGraph->GetGoodPlaceForNewNode();

            // Create a new event node
            EventNode = FEdGraphSchemaAction_K2NewNode::SpawnNode<UK2Node_ViewModelPropertyChanged>(
                TargetGraph,
                NewNodePos,
                EK2NewNodeFlags::SelectNewNode,
                [ViewModelClass, PropertyName](UK2Node_ViewModelPropertyChanged* NewInstance)
                {
                    NewInstance->ViewModelOwnerClass = ViewModelClass;
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

int32 FViewWidgetCustomizationExtender::HandleAddOrViewIndexForButton(UClass* ViewModelClass, FName PropertyName, UBlueprint* Blueprint) const
{
    UK2Node_ViewModelPropertyChanged* EventNode = FindEventNode(ViewModelClass, PropertyName, Blueprint);

    return EventNode ? 0 : 1;
}

UK2Node_ViewModelPropertyChanged* FViewWidgetCustomizationExtender::FindEventNode(UClass* ViewModelClass, FName PropertyName, UBlueprint* Blueprint) const
{
    TArray<UK2Node_ViewModelPropertyChanged*> EventNodes;
    FBlueprintEditorUtils::GetAllNodesOfClass(Blueprint, EventNodes);

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