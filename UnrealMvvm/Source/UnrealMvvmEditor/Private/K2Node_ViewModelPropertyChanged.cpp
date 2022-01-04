// Copyright Andrei Sudarikov. All Rights Reserved.

#include "K2Node_ViewModelPropertyChanged.h"
#include "Mvvm/BaseViewModel.h"
#include "Mvvm/Impl/MvvmBlueprintLibrary.h"
#include "EdGraphSchema_K2.h"
#include "KismetCompiler.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_CallFunction.h"
#include "K2Node_Self.h"
#include "GraphEditorSettings.h"
#include "ViewModelPropertyNodeHelper.h"

#define LOCTEXT_NAMESPACE "UnrealMvvm"

bool UK2Node_ViewModelPropertyChanged::Modify(bool bAlwaysMarkDirty)
{
    CachedNodeTitle.MarkDirty();

    return Super::Modify(bAlwaysMarkDirty);
}

void UK2Node_ViewModelPropertyChanged::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
    Super::ExpandNode(CompilerContext, SourceGraph);

    UEdGraphPin* ExecPin = FindPin(UEdGraphSchema_K2::PN_Then);
    UEdGraphPin* ValuePin = FindPin(ViewModelPropertyName);
    UEdGraphPin* HasValuePin = FindPin(FViewModelPropertyNodeHelper::HasValuePinName);

    if (ExecPin->LinkedTo.Num() > 0)
    {
        TSharedPtr<FUserPinInfo> UserPin = MakeShared<FUserPinInfo>();
        UserPin->DesiredPinDirection = EGPD_Output;
        UserPin->PinName = ViewModelPropertyName;

        if (FViewModelPropertyNodeHelper::FillPinType(UserPin->PinType, ViewModelPropertyName, ViewModelOwnerClass))
        {
            UK2Node_CustomEvent* CustomEvent = CompilerContext.SpawnIntermediateEventNode<UK2Node_CustomEvent>(this, ExecPin, SourceGraph);

            CustomEvent->CustomFunctionName = UnrealMvvm_Impl::FViewModelPropertyNamesCache::MakeCallbackName(ViewModelPropertyName);
            CustomEvent->AllocateDefaultPins();

            FViewModelPropertyNodeHelper::SpawnReadPropertyValueNodes(ValuePin, HasValuePin, CompilerContext, this, SourceGraph, ViewModelPropertyName);

            const UEdGraphSchema_K2* Schema = CompilerContext.GetSchema();
            CompilerContext.MovePinLinksToIntermediate(*ExecPin, *Schema->FindExecutionPin(*CustomEvent, EGPD_Output));
        }
    }
}

void UK2Node_ViewModelPropertyChanged::AllocateDefaultPins()
{
    CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);

    // create pin based on property type
    FEdGraphPinType PinType;
    const UnrealMvvm_Impl::FViewModelPropertyReflection* Property;
    if (FViewModelPropertyNodeHelper::FillPinType(PinType, ViewModelPropertyName, ViewModelOwnerClass, &Property))
    {
        CreatePin(EGPD_Output, PinType, ViewModelPropertyName);

        // create pin for TOptional HasValue
        if (Property->IsOptional)
        {
            CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Boolean, FViewModelPropertyNodeHelper::HasValuePinName);
        }
    }
}

FText UK2Node_ViewModelPropertyChanged::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
    if (CachedNodeTitle.IsOutOfDate(this))
    {
        FFormatNamedArguments Args;
        Args.Add(TEXT("PropertyName"), FText::FromName(ViewModelPropertyName));

        // FText::Format() is slow, so we cache this to save on performance
        CachedNodeTitle.SetCachedText(FText::Format(LOCTEXT("ViewModelPropertyChangedEvent_Title", "On {PropertyName} Changed"), Args), this);
    }

    return CachedNodeTitle;
}

FLinearColor UK2Node_ViewModelPropertyChanged::GetNodeTitleColor() const
{
    return GetDefault<UGraphEditorSettings>()->EventNodeTitleColor;
}

FSlateIcon UK2Node_ViewModelPropertyChanged::GetIconAndTint(FLinearColor& OutColor) const
{
    static FSlateIcon Icon("EditorStyle", "GraphEditor.Event_16x");
    return Icon;
}

#undef LOCTEXT_NAMESPACE