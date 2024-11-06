// Copyright Andrei Sudarikov. All Rights Reserved.

#include "SViewModelBindingEntry.h"
#include "Mvvm/ViewModelProperty.h"
#include "Nodes/K2Node_ViewModelPropertyChanged.h"
#include "Nodes/ViewModelPropertyNodeHelper.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "PropertyCustomizationHelpers.h"
#include "Styling/StyleColors.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Images/SLayeredImage.h"

void SViewModelBindingEntry::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView, UK2Node_ViewModelPropertyChanged* InNode)
{
    Node = InNode;
    RemoveDelegate = InArgs._OnRemoveClicked;

    auto GetBorderBackground = [this]()
    {
        return IsHovered() ?
            FAppStyle::Get().GetSlateColor("Colors.Header") :
            FAppStyle::Get().GetSlateColor("Colors.Panel");
    };

    ChildSlot
    [
        SNew( SBorder )
        .BorderImage(FAppStyle::Get().GetBrush("DetailsView.GridLine"))
        .Padding(0, 0, 0, 1)
        .Clipping(EWidgetClipping::ClipToBounds)
        [
            SNew(SBox)
            .MinDesiredHeight(26)
            [
                SNew( SBorder )
                .BorderImage(FAppStyle::Get().GetBrush("DetailsView.CategoryMiddle"))
                .BorderBackgroundColor_Lambda(GetBorderBackground)
                .Padding(8, 0, 16, 0)
                .VAlign(VAlign_Center)
                [
                    MakeContentWidget()
                ]
            ]
        ]
    ];
}

TSharedRef<SWidget> SViewModelBindingEntry::MakeContentWidget()
{
    TSharedRef<SHorizontalBox> Result =
        SNew(SHorizontalBox)

        // "View node" button
        + SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        .Padding(0, 0, 4, 0)
        [
            SNew(SButton)
            .HAlign(HAlign_Center)
            .ButtonStyle(FAppStyle::Get(), "SimpleButton")
            .ContentPadding(FMargin(2.0, 2.0))
            .ToolTipText(NSLOCTEXT("UnrealMvvm", "ViewEventTooltip", "View property changed handler"))
            .OnClicked(this, &ThisClass::HandleViewButtonClicked)
            [
                SNew(SImage)
                .ColorAndOpacity(FSlateColor::UseForeground())
                .Image(FAppStyle::Get().GetBrush("Icons.SelectInViewport"))
            ]
        ]

        // "Property path" widget
        + SHorizontalBox::Slot()
        .FillWidth(1)
        .VAlign(VAlign_Center)
        [
            MakePropertyPathWidget()
        ]

        // "Remove binding" button
        + SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        .Padding(0, 0, 4, 0)
        [
            PropertyCustomizationHelpers::MakeDeleteButton(RemoveDelegate)
        ]
    ;

    return Result;
}

TSharedRef<SWidget> SViewModelBindingEntry::MakePropertyPathWidget()
{
    TSharedRef<SWrapBox> WrapBox = SNew(SWrapBox)
        .Orientation(EOrientation::Orient_Horizontal)
        .UseAllottedSize(true);

    // iterate over all entries and create widget for each of them
    FViewModelPropertyNodeHelper::ForEachPropertyInPath(Node->PropertyPath, Node->GetViewModelClass(),
        [&](UClass* ViewModelClass, FName PropertyName, const UnrealMvvm_Impl::FViewModelPropertyReflection* Reflection)
    {
        AddPropertyPathEntry(*WrapBox, PropertyName, Reflection);
    });

    return WrapBox;
}

void SViewModelBindingEntry::AddPropertyPathEntry(SWrapBox& WrapBox, FName PropertyName, const UnrealMvvm_Impl::FViewModelPropertyReflection* Reflection)
{
    const UEdGraphSchema_K2* Schema = GetDefault<UEdGraphSchema_K2>();

    TSharedPtr<SWidget> PropertyIcon;
    if (Reflection != nullptr)
    {
        FEdGraphPinType PinType;
        FViewModelPropertyNodeHelper::FillPinType(PinType, Reflection);

        PropertyIcon = SNew(SLayeredImage, FBlueprintEditorUtils::GetSecondaryIconFromPin(PinType), Schema->GetSecondaryPinTypeColor(PinType))
            .Image(FBlueprintEditorUtils::GetIconFromPin(PinType, /* bIsLarge = */false))
            .ColorAndOpacity(Schema->GetPinTypeColor(PinType))
            .ToolTipText(Schema->TypeToText(PinType));
    }
    else
    {
        PropertyIcon = SNew(SImage)
            .ColorAndOpacity(FStyleColors::Error)
            .Image(FAppStyle::Get().GetBrush("Icons.Error"))
            .ToolTipText(INVTEXT("Invalid Property"));
    }

    TSharedRef<SHorizontalBox> HorizontalBox =
        SNew(SHorizontalBox)

        // ValueType icon
        + SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        .Padding(0, 0, 3, 0)
        [
            PropertyIcon.ToSharedRef()
        ]

        // Property name
        + SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        [
            SNew(STextBlock)
            .Text(FText::FromName(PropertyName))
        ];

    if (PropertyName != Node->PropertyPath.Last())
    {
        // add arrow
        HorizontalBox->AddSlot()
        .Padding(4, 0, 3, 0)
        [
            SNew(SImage)
            .ColorAndOpacity(FSlateColor::UseForeground())
            .Image(FAppStyle::Get().GetBrush("Icons.ChevronRight"))
        ];
    }

    WrapBox.AddSlot()[HorizontalBox];
}

FReply SViewModelBindingEntry::HandleViewButtonClicked()
{
    FKismetEditorUtilities::BringKismetToFocusAttentionOnObject(Node.Get());
    return FReply::Handled();
}
