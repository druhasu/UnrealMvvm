// Copyright Andrei Sudarikov. All Rights Reserved.

#include "BaseViewDetailCustomization.h"
#include "ViewModelPropertyNodeHelper.h"
#include "Mvvm/BaseView.h"
#include "Mvvm/BaseViewModel.h"
#include "Mvvm/Impl/ViewModelPropertyIterator.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"

#include "K2Node_ViewModelPropertyChanged.h"
#include "EdGraphSchema_K2_Actions.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "SourceCodeNavigation.h"

#include "Editor.h"
#include "Subsystems/AssetEditorSubsystem.h"

#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Layout/SWidgetSwitcher.h"

TSharedRef<IDetailCustomization> FBaseViewDetailCustomization::MakeInstance()
{
    return MakeShared<FBaseViewDetailCustomization>();
}

void FBaseViewDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
    static const FName CategoryName = "Base View Settings";
    IDetailCategoryBuilder& Category = DetailBuilder.EditCategory(CategoryName, FText::GetEmpty(), ECategoryPriority::TypeSpecific);

    TArray< TWeakObjectPtr<UObject> > OutObjects;
    DetailBuilder.GetObjectsBeingCustomized(OutObjects);

    if (OutObjects.Num() != 1)
    {
        // don't show additional category if editing multiple objects or none
        Category.SetCategoryVisibility(false);
        return;
    }

    UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
    TArray<UObject*> AllEditedAssets = AssetEditorSubsystem->GetAllEditedAssets();

    for (UObject* EditedAsset : AllEditedAssets)
    {
        UBlueprint* EditedBlueprint = Cast<UBlueprint>(EditedAsset);
        if (EditedBlueprint && EditedBlueprint->GeneratedClass == OutObjects[0]->GetClass())
        {
            Blueprint = EditedBlueprint;
        }
    }

    if (Blueprint == nullptr)
    {
        // don't show additional category if not editing selected object
        Category.SetCategoryVisibility(false);
        return;
    }

    UClass* ViewModelClass = UnrealMvvm_Impl::FViewModelRegistry::GetViewModelClass(OutObjects[0]->GetClass());
    UnrealMvvm_Impl::FViewModelPropertyIterator Iter(ViewModelClass, true);

    if (!Iter)
    {
        // don't show additional category if ViewModel has no properties
        Category.SetCategoryVisibility(false);
        return;
    }

    Category.SetCategoryVisibility(true);
    DetailBuilder.EditCategory(CategoryName, NSLOCTEXT("UnrealMvvm", "CategoryNameText","ViewModel Properties"), ECategoryPriority::TypeSpecific);

    // Add row with hyperlink to ViewModel source file
    FText ViewModelClassText = FText::FromString("ViewModel Class");
    Category.AddCustomRow(ViewModelClassText)
    .NameContent()
    [
        SNew(STextBlock)
        .Font(IDetailLayoutBuilder::GetDetailFont())
        .Text(ViewModelClassText)
    ]
    .ValueContent()
    .MinDesiredWidth(0)
    .MaxDesiredWidth(400)
    [
        SNew(SHyperlink)
        .Style(FEditorStyle::Get(), "Common.GotoNativeCodeHyperlink")
        .OnNavigate_Lambda([](UClass* C) { FSourceCodeNavigation::NavigateToClass(C); }, ViewModelClass)
        .Text(ViewModelClass->GetDisplayNameText())
        .ToolTipText(FText::Format(NSLOCTEXT("UnrealMvvm", "GotoViewModelSourceTooltip", "Click to open ViewModel source file in {0}"), FSourceCodeNavigation::GetSelectedSourceCodeIDE()))
    ];

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
                .Image(FEditorStyle::GetBrush("GraphEditor.Event_16x"))
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
                .ButtonStyle(FEditorStyle::Get(), "FlatButton.Success")
                .HAlign(HAlign_Center)
                .OnClicked(this, &FBaseViewDetailCustomization::HandleAddOrViewEventForProperty, ViewModelClass, PropertyName)
                .ForegroundColor(FSlateColor::UseForeground())
                [
                    SNew(SBox)
                    .MinDesiredHeight(14)
                    .VAlign(VAlign_Center)
                    [
                        SNew(SWidgetSwitcher)
                        .WidgetIndex(this, &FBaseViewDetailCustomization::HandleAddOrViewIndexForButton, ViewModelClass, PropertyName)
                        + SWidgetSwitcher::Slot()
                        [
                            SNew(STextBlock)
                            .Font(FEditorStyle::GetFontStyle(TEXT("BoldFont")))
                            .Text(NSLOCTEXT("UnrealMvvm", "ViewEvent", "View"))
                        ]
                        + SWidgetSwitcher::Slot()
                        [
                            SNew(SImage)
                            .Image(FEditorStyle::GetBrush("Plus"))
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

FReply FBaseViewDetailCustomization::HandleAddOrViewEventForProperty(UClass* ViewModelClass, FName PropertyName)
{
    UEdGraph* TargetGraph = Blueprint->GetLastEditedUberGraph();

    if (TargetGraph != nullptr)
    {
        UK2Node_ViewModelPropertyChanged* EventNode = FindEventNode(ViewModelClass, PropertyName);

        if(EventNode == nullptr)
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

int32 FBaseViewDetailCustomization::HandleAddOrViewIndexForButton(UClass* ViewModelClass, FName PropertyName) const
{
    UK2Node_ViewModelPropertyChanged* EventNode = FindEventNode(ViewModelClass, PropertyName);

    return EventNode ? 0 : 1;
}

UK2Node_ViewModelPropertyChanged* FBaseViewDetailCustomization::FindEventNode(UClass* ViewModelClass, FName PropertyName) const
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