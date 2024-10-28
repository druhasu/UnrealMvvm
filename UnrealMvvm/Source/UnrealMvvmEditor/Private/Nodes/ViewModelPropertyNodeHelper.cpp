// Copyright Andrei Sudarikov. All Rights Reserved.

#include "ViewModelPropertyNodeHelper.h"
#include "Mvvm/Impl/ViewModelPropertyIterator.h"
#include "Mvvm/BaseViewModel.h"
#include "Mvvm/MvvmBlueprintLibrary.h"
#include "Blueprint/UserWidget.h"
#include "KismetCompiler.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "EdGraph/EdGraphNode.h"
#include "BlueprintNodeSpawner.h"
#include "K2Node_CallFunction.h"
#include "K2Node_Self.h"

const FName FViewModelPropertyNodeHelper::HasValuePinName("HasValue");
const FName FViewModelPropertyNodeHelper::InputViewModelPinName("ViewModel");

bool FViewModelPropertyNodeHelper::IsPropertyAvailableInBlueprint(const UnrealMvvm_Impl::FViewModelPropertyReflection& Property)
{
    bool Result = Property.PinCategoryType != UnrealMvvm_Impl::EPinCategoryType::Unsupported;

    if (Result)
    {
        const bool bRequiresTypeHash = Property.ContainerType == EPinContainerType::Set || Property.ContainerType == EPinContainerType::Map;

        if (bRequiresTypeHash)
        {
            FEdGraphPinType Temp;
            Temp.PinCategory = GetPinCategoryNameFromType(Property.PinCategoryType);
            Temp.PinSubCategoryObject = Property.GetPinSubCategoryObject();

            Result &= FBlueprintEditorUtils::HasGetTypeHash(Temp);
        }

        if (Property.ContainerType == EPinContainerType::Map)
        {
            Result &= Property.PinValueCategoryType != UnrealMvvm_Impl::EPinCategoryType::Unsupported;

            // we support only UEnums. regular enum class won't have SubCategoryObject available
            if (Property.PinValueCategoryType == UnrealMvvm_Impl::EPinCategoryType::Enum)
            {
                Result &= Property.GetPinValueSubCategoryObject() != nullptr;
            }
        }

        // we support only UEnums. regular enum class won't have SubCategoryObject available
        if (Property.PinCategoryType == UnrealMvvm_Impl::EPinCategoryType::Enum)
        {
            Result &= Property.GetPinSubCategoryObject() != nullptr;
        }
    }

    return Result;
}

bool FViewModelPropertyNodeHelper::FillPinType(FEdGraphPinType& PinType, const FName& ViewModelPropertyName, UClass* ViewModelOwnerClass, const UnrealMvvm_Impl::FViewModelPropertyReflection** OutProperty)
{
    const UnrealMvvm_Impl::FViewModelPropertyReflection* Property = UnrealMvvm_Impl::FViewModelRegistry::FindProperty(ViewModelOwnerClass, ViewModelPropertyName);
    if (OutProperty)
    {
        *OutProperty = Property;
    }

    if (Property && IsPropertyAvailableInBlueprint(*Property))
    {
        PinType.PinCategory = GetPinCategoryNameFromType(Property->PinCategoryType);
        PinType.PinSubCategory = GetPinSubCategoryNameFromType(Property->PinCategoryType);
        PinType.PinSubCategoryObject = Property->GetPinSubCategoryObject();

        PinType.bIsConst = true;
        PinType.bIsReference = Property->ContainerType != EPinContainerType::None;
        PinType.ContainerType = Property->ContainerType;

        if (PinType.ContainerType == EPinContainerType::Map)
        {
            PinType.PinValueType.TerminalCategory = GetPinCategoryNameFromType(Property->PinValueCategoryType);
            PinType.PinValueType.TerminalSubCategory = GetPinSubCategoryNameFromType(Property->PinValueCategoryType);
            PinType.PinValueType.TerminalSubCategoryObject = Property->GetPinValueSubCategoryObject();
        }

        return true;
    }

    return false;
}

FName FViewModelPropertyNodeHelper::GetPinCategoryNameFromType(UnrealMvvm_Impl::EPinCategoryType CategoryType)
{
    #define PIN_CASE(PinType) \
        case UnrealMvvm_Impl::EPinCategoryType:: PinType: \
            return UEdGraphSchema_K2::PC_##PinType

    switch (CategoryType)
    {
        case UnrealMvvm_Impl::EPinCategoryType::Unsupported:
        default:
            return FName();

        PIN_CASE(Boolean);
        PIN_CASE(Byte);
        PIN_CASE(Class);
        PIN_CASE(SoftClass);
        PIN_CASE(Int);
        PIN_CASE(Int64);
#if ENGINE_MAJOR_VERSION < 5
        PIN_CASE(Float);
#endif
        PIN_CASE(Name);
        PIN_CASE(Object);
        PIN_CASE(Interface);
        PIN_CASE(SoftObject);
        PIN_CASE(String);
        PIN_CASE(Text);
        PIN_CASE(Struct);

        // special handling for Enum. Unreal uses Byte for them
        case UnrealMvvm_Impl::EPinCategoryType::Enum:
            return UEdGraphSchema_K2::PC_Byte;

        // special handling for floats and doubles in UE5
#if ENGINE_MAJOR_VERSION >= 5
        case UnrealMvvm_Impl::EPinCategoryType::Float:
        case UnrealMvvm_Impl::EPinCategoryType::Double:
            return UEdGraphSchema_K2::PC_Real;
#endif
    }

    #undef PIN_CASE
}

FName FViewModelPropertyNodeHelper::GetPinSubCategoryNameFromType(UnrealMvvm_Impl::EPinCategoryType CategoryType)
{
    switch (CategoryType)
    {
        case UnrealMvvm_Impl::EPinCategoryType::Unsupported:
        default:
            return FName();

        // only Floats and Doubles in UE5 are using subcategory name right now
#if ENGINE_MAJOR_VERSION >= 5
        case UnrealMvvm_Impl::EPinCategoryType::Float:
            return UEdGraphSchema_K2::PC_Float;
        case UnrealMvvm_Impl::EPinCategoryType::Double:
            return UEdGraphSchema_K2::PC_Double;
#endif
    }
}

void FViewModelPropertyNodeHelper::SpawnGetSetPropertyValueNodes(const FName& FunctionName, FKismetCompilerContext& CompilerContext, UEdGraphNode* SourceNode, UEdGraph* SourceGraph, const FName& ViewModelPropertyName)
{
    UEdGraphPin* ValuePin = SourceNode->FindPin(ViewModelPropertyName);
    if (ValuePin == nullptr)
    {
        // if SourceNode is referencing a removed property, the pin will not exist
        // no need to expand anything in this case
        return;
    }

    const UEdGraphSchema_K2* Schema = CompilerContext.GetSchema();

    UK2Node_CallFunction* GetSetViewModelPropertyValueCall = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(SourceNode, SourceGraph);
    GetSetViewModelPropertyValueCall->FunctionReference.SetExternalMember(FunctionName, UMvvmBlueprintLibrary::StaticClass());
    GetSetViewModelPropertyValueCall->AllocateDefaultPins();

    UK2Node_Self* Self = CompilerContext.SpawnIntermediateNode<UK2Node_Self>(SourceNode, SourceGraph);
    Self->AllocateDefaultPins();

    UEdGraphPin* SelfOutPin = Self->FindPin(UEdGraphSchema_K2::PN_Self);
    UEdGraphPin* ViewInPin = GetSetViewModelPropertyValueCall->FindPin(TEXT("View"));
    Schema->TryCreateConnection(SelfOutPin, ViewInPin);

    ConnectOutputPins(GetSetViewModelPropertyValueCall, CompilerContext, SourceNode, SourceGraph, Schema, ValuePin, ViewModelPropertyName);
}

void FViewModelPropertyNodeHelper::SpawnExplicitGetSetPropertyValueNodes(const FName& FunctionName, FKismetCompilerContext& CompilerContext, UEdGraphNode* SourceNode, UEdGraph* SourceGraph, const FName& ViewModelPropertyName)
{
    UEdGraphPin* ValuePin = SourceNode->FindPin(ViewModelPropertyName);
    if (ValuePin == nullptr)
    {
        // if SourceNode is referencing a removed property, the pin will not exist
        // no need to expand anything in this case
        return;
    }

    const UEdGraphSchema_K2* Schema = CompilerContext.GetSchema();
    UEdGraphPin* InputViewModelPin = SourceNode->FindPin(InputViewModelPinName);
    if (!InputViewModelPin || InputViewModelPin->LinkedTo.IsEmpty())
    {
        return;
    }

    UK2Node_CallFunction* GetSetViewModelPropertyValueCall =
        CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(SourceNode, SourceGraph);

    GetSetViewModelPropertyValueCall->FunctionReference.SetExternalMember(FunctionName, UMvvmBlueprintLibrary::StaticClass());
    GetSetViewModelPropertyValueCall->AllocateDefaultPins();

    UEdGraphPin* GetSetViewModelInPin = GetSetViewModelPropertyValueCall->FindPin(TEXT("ViewModel"));
    for (UEdGraphPin* LinkedOutPin : InputViewModelPin->LinkedTo)
    {
        check(LinkedOutPin->Direction == EGPD_Output);
        Schema->TryCreateConnection(LinkedOutPin, GetSetViewModelInPin);
    }

    ConnectOutputPins(GetSetViewModelPropertyValueCall, CompilerContext, SourceNode, SourceGraph, Schema, ValuePin, ViewModelPropertyName);
}

void FViewModelPropertyNodeHelper::AddInputViewModelPin(UEdGraphNode& Node, UClass* ViewModelClass)
{
    // create input pin for ViewModel parameter
    FEdGraphPinType InputPinType;
    InputPinType.PinCategory = UEdGraphSchema_K2::PC_Object;
    InputPinType.PinSubCategoryObject = ViewModelClass;
    InputPinType.bIsConst = true;

    Node.CreatePin(EGPD_Input, InputPinType, InputViewModelPinName);
}

bool FViewModelPropertyNodeHelper::IsActionValidInContext(const FBlueprintActionFilter& Filter, const UClass* ViewModelOwnerClass)
{
    bool bFoundCompatible = false;

    // Prevent this node from showing up in Blueprints that do not have appropriate ViewModel
    for (UBlueprint* Blueprint : Filter.Context.Blueprints)
    {
        UClass* ViewClass = Blueprint->GeneratedClass;
        UClass* ViewModelClass = UnrealMvvm_Impl::FViewModelRegistry::GetViewModelClass(ViewClass);

        if (ViewModelClass && ViewModelOwnerClass->IsChildOf(ViewModelClass))
        {
            bFoundCompatible = true;
        }
        else
        {
            if (Filter.Context.Pins.IsEmpty())
            {
                // Check all nested ViewModel fields of our ViewModel
                for (UnrealMvvm_Impl::FViewModelPropertyIterator It(ViewModelClass, false); It; ++It)
                {
                    const UnrealMvvm_Impl::FViewModelPropertyReflection& Property = *It;
                    if (Property.PinCategoryType == UnrealMvvm_Impl::EPinCategoryType::Object ||
                        Property.PinCategoryType == UnrealMvvm_Impl::EPinCategoryType::Interface ||
                        Property.PinCategoryType == UnrealMvvm_Impl::EPinCategoryType::SoftObject)
                    {
                        if (UClass* NestedPropertyClass = Cast<UClass>(Property.GetPinSubCategoryObject()); NestedPropertyClass && ViewModelOwnerClass->IsChildOf(NestedPropertyClass))
                        {
                            bFoundCompatible = true;
                            break;
                        }
                    }
                }
            }
            else
            {
                // Search currently selected pins for a corresponding ViewModel class
                for (const UEdGraphPin* Pin : Filter.Context.Pins)
                {
                    const FEdGraphPinType& PinType = Pin->PinType;
                    if (PinType.PinCategory == UEdGraphSchema_K2::PC_Object &&
                        PinType.PinSubCategoryObject.IsValid() &&
                        PinType.PinSubCategoryObject == ViewModelOwnerClass)
                    {
                        bFoundCompatible = true;
                        break;
                    }
                }
            }
        }
    }

    return bFoundCompatible;
}

void FViewModelPropertyNodeHelper::ValidateInputViewModelPin(const UEdGraphNode* Node, FCompilerResultsLog& MessageLog)
{
    UEdGraphPin* InputPin = Node->FindPin(InputViewModelPinName);
    check(InputPin);
    if (!InputPin->HasAnyConnections())
    {
        FText Message = NSLOCTEXT("UnrealMvvm", "Error.InputViewModelIsRequired", "Missing input ViewModel for node @@");
        MessageLog.Error(*Message.ToString(), Node);
    }
}

FName FViewModelPropertyNodeHelper::GetFunctionNameForGetPropertyValue(UClass* ViewClass)
{
    static const FName GetFromWidgetName(GET_MEMBER_NAME_CHECKED(UMvvmBlueprintLibrary, GetViewModelPropertyValueFromWidget));
    static const FName GetFromActorName(GET_MEMBER_NAME_CHECKED(UMvvmBlueprintLibrary, GetViewModelPropertyValueFromActor));

    return ViewClass->IsChildOf<UUserWidget>() ? GetFromWidgetName : GetFromActorName;
}

FName FViewModelPropertyNodeHelper::GetFunctionNameForSetPropertyValue(UClass* ViewClass)
{
    static const FName SetToWidgetName(GET_MEMBER_NAME_CHECKED(UMvvmBlueprintLibrary, SetViewModelPropertyValueToWidget));
    static const FName SetToActorName(GET_MEMBER_NAME_CHECKED(UMvvmBlueprintLibrary, SetViewModelPropertyValueToActor));

    return ViewClass->IsChildOf<UUserWidget>() ? SetToWidgetName : SetToActorName;
}

FName FViewModelPropertyNodeHelper::GetFunctionNameForGetPropertyValue()
{
    static const FName GetFromViewModelName(GET_MEMBER_NAME_CHECKED(UMvvmBlueprintLibrary, GetViewModelPropertyValue));
    return GetFromViewModelName;
}

FName FViewModelPropertyNodeHelper::GetFunctionNameForSetPropertyValue()
{
    static const FName SetFromViewModelName(GET_MEMBER_NAME_CHECKED(UMvvmBlueprintLibrary, SetViewModelPropertyValue));
    return SetFromViewModelName;
}

void FViewModelPropertyNodeHelper::ConnectOutputPins(UK2Node_CallFunction* GetSetViewModelPropertyValueCall, FKismetCompilerContext& CompilerContext, UEdGraphNode* SourceNode, UEdGraph* SourceGraph, const UEdGraphSchema_K2* Schema, UEdGraphPin* ValuePin, const FName& ViewModelPropertyName)
{
    UEdGraphPin* PropertyNamePin = GetSetViewModelPropertyValueCall->FindPin(TEXT("PropertyName"));
    PropertyNamePin->DefaultValue = ViewModelPropertyName.ToString();

    UEdGraphPin* ValueOutPin = GetSetViewModelPropertyValueCall->FindPin(TEXT("Value"));
    ValueOutPin->PinType.PinCategory = ValuePin->PinType.PinCategory;
    ValueOutPin->PinType.PinValueType = ValuePin->PinType.PinValueType;
    ValueOutPin->PinType.PinSubCategory = ValuePin->PinType.PinSubCategory;
    ValueOutPin->PinType.PinSubCategoryObject = ValuePin->PinType.PinSubCategoryObject;
    ValueOutPin->PinType.ContainerType = ValuePin->PinType.ContainerType;

    CompilerContext.MovePinLinksToIntermediate(*ValuePin, *ValueOutPin);

    UEdGraphPin* SourceExecutePin = Schema->FindExecutionPin(*SourceNode, EGPD_Input);
    UEdGraphPin* SourceThenPin = Schema->FindExecutionPin(*SourceNode, EGPD_Output);
    if (SourceExecutePin && SourceThenPin)
    {
        CompilerContext.MovePinLinksToIntermediate(*SourceExecutePin, *Schema->FindExecutionPin(*GetSetViewModelPropertyValueCall, EGPD_Input));
        CompilerContext.MovePinLinksToIntermediate(*SourceThenPin, *Schema->FindExecutionPin(*GetSetViewModelPropertyValueCall, EGPD_Output));
    }

    UEdGraphPin* HasValuePin = SourceNode->FindPin(FViewModelPropertyNodeHelper::HasValuePinName);
    if (HasValuePin)
    {
        UEdGraphPin* HasValueOutPin = GetSetViewModelPropertyValueCall->FindPin(HasValuePinName);
        CompilerContext.MovePinLinksToIntermediate(*HasValuePin, *HasValueOutPin);
    }
}