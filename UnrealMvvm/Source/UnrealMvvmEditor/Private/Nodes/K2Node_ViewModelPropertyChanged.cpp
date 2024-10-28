// Copyright Andrei Sudarikov. All Rights Reserved.

#include "K2Node_ViewModelPropertyChanged.h"
#include "Mvvm/BaseViewModel.h"
#include "Mvvm/MvvmBlueprintLibrary.h"
#include "EdGraphSchema_K2.h"
#include "KismetCompiler.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_CallFunction.h"
#include "K2Node_Self.h"
#include "GraphEditorSettings.h"
#include "ViewModelPropertyNodeHelper.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Misc/EngineVersionComparison.h"

void UK2Node_ViewModelPropertyChanged::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
    Super::ExpandNode(CompilerContext, SourceGraph);

    UEdGraphPin* ExecPin = FindPin(UEdGraphSchema_K2::PN_Then);

    if (ExecPin->LinkedTo.Num() > 0)
    {
        FEdGraphPinType PinType;

        if (FViewModelPropertyNodeHelper::FillPinType(PinType, ViewModelPropertyName, ViewModelOwnerClass))
        {
#if UE_VERSION_OLDER_THAN(5,5,0)
            UK2Node_CustomEvent* CustomEvent = CompilerContext.SpawnIntermediateEventNode<UK2Node_CustomEvent>(this, ExecPin, SourceGraph);
#else
            UK2Node_CustomEvent* CustomEvent = CompilerContext.SpawnIntermediateNode<UK2Node_CustomEvent>(this, SourceGraph);
#endif

            CustomEvent->CustomFunctionName = UnrealMvvm_Impl::FViewModelPropertyNamesCache::MakeCallbackName(ViewModelPropertyName);
            CustomEvent->AllocateDefaultPins();

            FName FunctionName = FViewModelPropertyNodeHelper::GetFunctionNameForGetPropertyValue(FBlueprintEditorUtils::FindBlueprintForNodeChecked(this)->GeneratedClass);
            FViewModelPropertyNodeHelper::SpawnGetSetPropertyValueNodes(FunctionName, CompilerContext, this, SourceGraph, ViewModelPropertyName);

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
        if (Property->Flags.IsOptional)
        {
            CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Boolean, FViewModelPropertyNodeHelper::HasValuePinName);
        }
    }
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

FText UK2Node_ViewModelPropertyChanged::GetNodeTitleForCache(ENodeTitleType::Type TitleType) const
{
    return FText::Format(NSLOCTEXT("UnrealMvvm", "ViewModelPropertyChangedEvent_Title", "On {0} Changed"), FText::FromName(ViewModelPropertyName));
}