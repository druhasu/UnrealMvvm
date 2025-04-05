// Copyright Andrei Sudarikov. All Rights Reserved.

#include "K2Node_ViewModelPropertyChanged.h"
#include "Mvvm/Impl/BaseView/ViewRegistry.h"
#include "Mvvm/MvvmBlueprintLibrary.h"
#include "ViewModelClassSelectorHelper.h"
#include "Blueprint/UserWidget.h"
#include "EdGraphSchema_K2.h"
#include "KismetCompiler.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_CallFunction.h"
#include "GraphEditorSettings.h"
#include "ViewModelPropertyNodeHelper.h"
#include "Misc/EngineVersionComparison.h"

const FName UK2Node_ViewModelPropertyChanged::IsInitialPinName(TEXT("IsInitial"));

void UK2Node_ViewModelPropertyChanged::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
    Super::ExpandNode(CompilerContext, SourceGraph);

    if (!FViewModelPropertyNodeHelper::IsPropertyPathValid(PropertyPath, GetViewModelClass()))
    {
        // don't expand if property path is not valid
        return;
    }

    UEdGraphPin* ExecPin = FindPin(UEdGraphSchema_K2::PN_Then);
    if (ExecPin->LinkedTo.Num() == 0)
    {
        // don't expand if nothing is connected
        return;
    }

    const UEdGraphSchema_K2* Schema = CompilerContext.GetSchema();

    // Spawn custom event node to create a function for callback
#if UE_VERSION_OLDER_THAN(5,4,0)
    UK2Node_CustomEvent* CustomEvent = CompilerContext.SpawnIntermediateEventNode<UK2Node_CustomEvent>(this, ExecPin, SourceGraph);
#else
    UK2Node_CustomEvent* CustomEvent = CompilerContext.SpawnIntermediateNode<UK2Node_CustomEvent>(this, SourceGraph);
#endif
    CustomEvent->CustomFunctionName = MakeCallbackName();
    CustomEvent->AllocateDefaultPins();

    // connect "then" pin of this node to "exec" pin of CustomEvent node
    CompilerContext.MovePinLinksToIntermediate(*ExecPin, *Schema->FindExecutionPin(*CustomEvent, EGPD_Output));

    UEdGraphPin* ValueOutPin = FindPin(PropertyPath.Last());
    UEdGraphPin* HasValueOutPin = FindPin(FViewModelPropertyNodeHelper::HasValuePinName);

    // don't spawn GetPropertyValue nodes if result pin of this node is not connected
    if (ValueOutPin->HasAnyConnections() || (HasValueOutPin != nullptr && HasValueOutPin->HasAnyConnections()))
    {
        // spawn GetViewModel node
        UK2Node_CallFunction* GetViewModelCall = FViewModelPropertyNodeHelper::SpawnGetViewModelNodes(CompilerContext, this, SourceGraph);
        UEdGraphPin* LastReturnValuePin = GetViewModelCall->GetReturnValuePin();

        // spawn Get Value nodes for all properties in the path
        FViewModelPropertyNodeHelper::ForEachPropertyInPath(PropertyPath, GetViewModelClass(),
            [&](UClass* ViewModelClass, FName PropertyName, const UnrealMvvm_Impl::FViewModelPropertyReflection* Reflection)
        {
            // spawn GetViewModelPropertyValue
            UK2Node_CallFunction* GetViewModelPropertyValueCall = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
            GetViewModelPropertyValueCall->FunctionReference.SetExternalMember(FViewModelPropertyNodeHelper::GetPropertyValueFunctionName, UMvvmBlueprintLibrary::StaticClass());
            GetViewModelPropertyValueCall->AllocateDefaultPins();

            // set property name
            UEdGraphPin* PropertyNamePin = GetViewModelPropertyValueCall->FindPin(TEXT("PropertyName"));
            PropertyNamePin->DefaultValue = PropertyName.ToString();

            // set correct type to ReturnValue pin
            UEdGraphPin* ReturnValuePin = GetViewModelPropertyValueCall->FindPin(TEXT("Value"));
            FViewModelPropertyNodeHelper::FillPinType(ReturnValuePin->PinType, PropertyName, ViewModelClass);

            // connect ViewModel output pin of previous node to ViewModel input of a new node
            UEdGraphPin* ViewModelInPin = GetViewModelPropertyValueCall->FindPin(FViewModelPropertyNodeHelper::ViewModelPinName);
            Schema->TryCreateConnection(LastReturnValuePin, ViewModelInPin);

            // update last ReturnValue pin to a new one
            LastReturnValuePin = ReturnValuePin;
        });

        // connect ReturnValue pin of a last GetViewModelPropertyValue node to the output of this node
        CompilerContext.MovePinLinksToIntermediate(*ValueOutPin, *LastReturnValuePin);

        // connect HasValue pin if exists
        if (HasValueOutPin)
        {
            // find HasValue pin in the last node
            UEdGraphPin* LastHasValuePin = LastReturnValuePin->GetOwningNode()->FindPin(FViewModelPropertyNodeHelper::HasValuePinName);
            CompilerContext.MovePinLinksToIntermediate(*HasValueOutPin, *LastHasValuePin);
        }
    }

    // don't spawn nodes if not connected
    UEdGraphPin* IsInitialPin = FindPin(IsInitialPinName);
    if (IsInitialPin != nullptr && IsInitialPin->HasAnyConnections())
    {
        static const FName IsInitializingWidget = GET_MEMBER_NAME_CHECKED(UMvvmBlueprintLibrary, IsInitializingPropertyInWidget);
        static const FName IsInitializingActor = GET_MEMBER_NAME_CHECKED(UMvvmBlueprintLibrary, IsInitializingPropertyInActor);

        FName IsInitializingFunctionName = GetBlueprint()->GeneratedClass->IsChildOf<UUserWidget>()
            ? IsInitializingWidget
            : IsInitializingActor;

        // spawn call to appropriate IsInitializing variant
        UK2Node_CallFunction* IsInitializingCall = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
        IsInitializingCall->FunctionReference.SetExternalMember(IsInitializingFunctionName, UMvvmBlueprintLibrary::StaticClass());
        IsInitializingCall->AllocateDefaultPins();

        CompilerContext.MovePinLinksToIntermediate(*IsInitialPin, *IsInitializingCall->GetReturnValuePin());
    }
}

void UK2Node_ViewModelPropertyChanged::AllocateDefaultPins()
{
    CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);

    // Find Reflection info of the last Property in path
    const UnrealMvvm_Impl::FViewModelPropertyReflection* Reflection = nullptr;
    FViewModelPropertyNodeHelper::ForEachPropertyInPath(PropertyPath, GetViewModelClass(),
        [&](UClass* ViewModelClass, FName PropertyName, const UnrealMvvm_Impl::FViewModelPropertyReflection* InReflection)
    {
        Reflection = InReflection;
    });

    // Property will be nullptr if PropertyPath is not valid
    if (Reflection != nullptr)
    {
        FEdGraphPinType PinType;
        FViewModelPropertyNodeHelper::FillPinType(PinType, Reflection);

        // create Value pin based on last property in Path
        CreatePin(EGPD_Output, PinType, Reflection->GetProperty()->GetName());

        // create pin for TOptional HasValue
        if (Reflection->Flags.IsOptional)
        {
            CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Boolean, FViewModelPropertyNodeHelper::HasValuePinName);
        }

        // create pin for easier access to IsInitializing function
        UEdGraphPin* IsInitialPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Boolean, TEXT("IsInitial"));
        IsInitialPin->PinToolTip = TEXT("Whether value is changed because ViewModel is first set to View (true), or value changed inside ViewModel after it was set (false)");
    }
}

void UK2Node_ViewModelPropertyChanged::ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const
{
    Super::ValidateNodeDuringCompilation(MessageLog);

    FViewModelPropertyNodeHelper::ForEachPropertyInPath(PropertyPath, GetViewModelClass(),
        [&](UClass* ViewModelClass, FName PropertyName, const UnrealMvvm_Impl::FViewModelPropertyReflection* Reflection)
    {
        if (Reflection == nullptr)
        {
            FText Message = FText::FormatOrdered(
                NSLOCTEXT("UnrealMvvm", "Error.PropertyPathInvalid.PropertyNotExist", "Invalid Property path in node @@. '{1}' does not contain property '{0}'"),
                FText::FromName(PropertyName),
                FText::FromString(GetNameSafe(ViewModelClass))
            );

            MessageLog.Error(*Message.ToString(), this);
        }
        else if (PropertyName != PropertyPath.Last() && Reflection->ContainerType != EPinContainerType::None)
        {
            FText Message = FText::FormatOrdered(
                NSLOCTEXT("UnrealMvvm", "Error.PropertyPathInvalid.PropertyIsContainer", "Invalid Property path in node @@. Property '{1}'.'{0}' is a container, but not last in path"),
                FText::FromName(PropertyName),
                FText::FromString(GetNameSafe(ViewModelClass))
            );

            MessageLog.Error(*Message.ToString(), this);
        }
    });
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

void UK2Node_ViewModelPropertyChanged::Serialize(FArchive& Ar)
{
    Super::Serialize(Ar);

    if (Ar.IsLoading())
    {
        // convert from legacy "single property" to a "property path" bindings
        if (PropertyPath.IsEmpty() && !ViewModelPropertyName_DEPRECATED.IsNone())
        {
            PropertyPath.Add(ViewModelPropertyName_DEPRECATED);
        }
    }
}

FName UK2Node_ViewModelPropertyChanged::MakeCallbackName() const
{
    TStringBuilderWithBuffer<TCHAR, 128> Builder;
    Builder.Append(TEXT("OnVM_"));

    // add name of current class to allow binding to the same property path from derived classes
    // derived classes will have different callback name this way
    GetBlueprint()->GetFName().AppendString(Builder);

    for (const FName& PropertyName : PropertyPath)
    {
        Builder.AppendChar(TEXT('_'));
        PropertyName.AppendString(Builder);
    }

    return FName(*Builder);
}

UClass* UK2Node_ViewModelPropertyChanged::GetViewModelClass() const
{
    return FViewModelClassSelectorHelper::GetViewModelClass(GetBlueprint());
}

FText UK2Node_ViewModelPropertyChanged::GetNodeTitleForCache(ENodeTitleType::Type TitleType) const
{
    return FText::Format(NSLOCTEXT("UnrealMvvm", "ViewModelPropertyChangedEvent_Title", "On {0} Changed"), FText::FromName(PropertyPath.Last()));
}

FText UK2Node_ViewModelPropertyChanged::GetTooltipTextForCache() const
{
    TStringBuilder<128> Builder;
    Builder.Append("Called when any property in the path changes:\n\n");

    FViewModelPropertyNodeHelper::ForEachPropertyInPath(PropertyPath, GetViewModelClass(),
        [&](UClass* ViewModelClass, FName PropertyName, auto)
    {
        Builder.Append(ViewModelClass->GetName());
        Builder.Append(TEXT("::"));
        PropertyName.AppendString(Builder);

        if (PropertyName != PropertyPath.Last())
        {
            Builder.Append(TEXT(" → "));
        }
    });

    return FText::FromStringView(Builder.ToView());
}
