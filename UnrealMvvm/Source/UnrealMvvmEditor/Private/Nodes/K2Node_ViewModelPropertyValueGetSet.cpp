// Copyright Andrei Sudarikov. All Rights Reserved.

#include "K2Node_ViewModelPropertyValueGetSet.h"
#include "ViewModelPropertyNodeHelper.h"

void UK2Node_ViewModelPropertyValueGetSet::PostPlacedNewNode()
{
    Super::PostPlacedNewNode();

    if (!FViewModelPropertyNodeHelper::IsBlueprintViewModelCompatible(this, ViewModelOwnerClass))
    {
        // make sure ViewModel pin is visible if node is placed in the incompatible Blueprint
        UEdGraphPin* ViewModelPin = FindPin(FViewModelPropertyNodeHelper::ViewModelPinName);
        if (ViewModelPin != nullptr)
        {
            ViewModelPin->SafeSetHidden(false);
        }

        bShowViewModelPin = true;
    }
}

void UK2Node_ViewModelPropertyValueGetSet::NotifyPinConnectionListChanged(UEdGraphPin* Pin)
{
    Super::NotifyPinConnectionListChanged(Pin);

    if (Pin->PinName == FViewModelPropertyNodeHelper::ViewModelPinName && Pin->HasAnyConnections())
    {
        Pin->SafeSetHidden(false);
        bShowViewModelPin = true;
    }
}

void UK2Node_ViewModelPropertyValueGetSet::ReconstructNode()
{
    Super::ReconstructNode();

    UEdGraphPin* ViewModelPin = FindPin(FViewModelPropertyNodeHelper::ViewModelPinName);
    if (ViewModelPin != nullptr && ViewModelPin->HasAnyConnections())
    {
        ViewModelPin->SafeSetHidden(false);
        bShowViewModelPin = true;
    }
}

void UK2Node_ViewModelPropertyValueGetSet::ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const
{
    Super::ValidateNodeDuringCompilation(MessageLog);

    // check that ViewModel pin is either connected or Blueprint has compatible ViewModel
    UEdGraphPin* ViewModelPin = FindPin(FViewModelPropertyNodeHelper::ViewModelPinName);
    check(ViewModelPin);

    if (!ViewModelPin->HasAnyConnections() && !FViewModelPropertyNodeHelper::IsBlueprintViewModelCompatible(this, ViewModelOwnerClass))
    {
        FText Message = NSLOCTEXT("UnrealMvvm", "Error.InputViewModelIsRequired", "Missing input ViewModel for node @@");
        MessageLog.Error(*Message.ToString(), this);
    }
}

void UK2Node_ViewModelPropertyValueGetSet::GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
    Super::GetNodeContextMenuActions(Menu, Context);

    if (!Context->bIsDebugging)
    {
        FText MenuEntryLabel = NSLOCTEXT("UnrealMvvm", "PropertyValueNode.ShowPinTitle", "Show ViewModel pin");
        FText MenuEntryTooltip = NSLOCTEXT("UnrealMvvm", "PropertyValueNode.ShowPinTooltip", "Show explicit ViewModel pin to be able to connect any ViewModel");

        bool bCanToggleViewModelPin = true;
        auto CanExecuteToggleViewModelPin = [](bool const bInCanToggleViewModelPin)->bool
        {
            return bInCanToggleViewModelPin;
        };

        if (bShowViewModelPin)
        {
            MenuEntryLabel = NSLOCTEXT("UnrealMvvm", "PropertyValueNode.HidePinTitle", "Hide ViewModel pin");
            MenuEntryTooltip = NSLOCTEXT("UnrealMvvm", "PropertyValueNode.HidePinTooltip", "Hide explicit ViewModel pin to use ViewModel of current View");

            if (FindPin(FViewModelPropertyNodeHelper::ViewModelPinName)->HasAnyConnections())
            {
                // disable "Hide" menu entry
                bCanToggleViewModelPin = false;
                MenuEntryTooltip = NSLOCTEXT("UnrealMvvm", "PropertyValueNode.HidePinTooltip_Connected", "Cannot Hide explicit ViewModel pin because it is connected");
            }

            if (!FViewModelPropertyNodeHelper::IsBlueprintViewModelCompatible(this, ViewModelOwnerClass))
            {
                bCanToggleViewModelPin = false;
                MenuEntryTooltip = NSLOCTEXT("UnrealMvvm", "PropertyValueNode.HidePinTooltip_BlueprintIncompatible", "Cannot Hide explicit ViewModel pin because current Blueprint has incompatible ViewModel");
            }
        }

        FToolMenuSection& Section = Menu->AddSection(GetFName(), NSLOCTEXT("UnrealMvvm", "PropertyValueNode.MenuSectionTitle", "ViewModel property value"));
        Section.AddMenuEntry(
            "ToggleViewModelPin",
            MenuEntryLabel,
            MenuEntryTooltip,
            FSlateIcon(),
            FUIAction(
                FExecuteAction::CreateUObject(const_cast<ThisClass*>(this), &ThisClass::ToggleViewModelPin),
                FCanExecuteAction::CreateStatic(CanExecuteToggleViewModelPin, bCanToggleViewModelPin),
                FIsActionChecked()
            ));
    }
}

void UK2Node_ViewModelPropertyValueGetSet::ToggleViewModelPin()
{
    const FText TransactionTitle = bShowViewModelPin ?
        NSLOCTEXT("UnrealMvvm", "PropertyValueNode.HidePinTransaction", "Hide ViewModel pin") :
        NSLOCTEXT("UnrealMvvm", "PropertyValueNode.ShowPinTransaction", "Show ViewModel pin");

    const FScopedTransaction Transaction(TransactionTitle);
    Modify();

    bShowViewModelPin = !bShowViewModelPin;

    const bool bHasBeenConstructed = Pins.Num() > 0;
    if (bHasBeenConstructed)
    {
        FindPin(FViewModelPropertyNodeHelper::ViewModelPinName)->SafeSetHidden(!bShowViewModelPin);
        ReconstructNode();
    }
}
