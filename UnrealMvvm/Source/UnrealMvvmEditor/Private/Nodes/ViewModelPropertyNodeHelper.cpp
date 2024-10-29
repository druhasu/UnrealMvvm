// Copyright Andrei Sudarikov. All Rights Reserved.

#include "ViewModelPropertyNodeHelper.h"
#include "Mvvm/Impl/Property/ViewModelPropertyIterator.h"
#include "Mvvm/Impl/BaseView/ViewRegistry.h"
#include "Mvvm/MvvmBlueprintLibrary.h"
#include "Blueprint/UserWidget.h"
#include "KismetCompiler.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "EdGraph/EdGraphNode.h"
#include "K2Node_CallFunction.h"
#include "K2Node_Self.h"

const FName FViewModelPropertyNodeHelper::HasValuePinName("HasValue");
const FName FViewModelPropertyNodeHelper::ViewModelPinName("ViewModel");
const FName FViewModelPropertyNodeHelper::ViewPinName("View");

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

    UEdGraphPin* GetSetViewModelPropertyValueInPin = GetSetViewModelPropertyValueCall->FindPin(ViewModelPinName);
    UEdGraphPin* InputViewModelPin = SourceNode->FindPin(ViewModelPinName);
    if (InputViewModelPin != nullptr && InputViewModelPin->HasAnyConnections())
    {
        // use connections from ViewModel pin
        for (UEdGraphPin* LinkedOutPin : InputViewModelPin->LinkedTo)
        {
            check(LinkedOutPin->Direction == EGPD_Output);
            Schema->TryCreateConnection(LinkedOutPin, GetSetViewModelPropertyValueInPin);
        }
    }
    else
    {
        // get ViewModel from current View: GetSelf -> GetViewModelFromWidget/GetViewModelFromActor
        UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForNodeChecked(SourceNode);
        FName GetViewModelFunctionName = Blueprint->GeneratedClass->IsChildOf<UUserWidget>() ?
            GET_MEMBER_NAME_CHECKED(UMvvmBlueprintLibrary, GetViewModelFromWidget) :
            GET_MEMBER_NAME_CHECKED(UMvvmBlueprintLibrary, GetViewModelFromActor);

        // GetViewModelFromWidget() / GetViewModelFromActor()
        UK2Node_CallFunction* GetViewModelCall = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(SourceNode, SourceGraph);
        GetViewModelCall->FunctionReference.SetExternalMember(GetViewModelFunctionName, UMvvmBlueprintLibrary::StaticClass());
        GetViewModelCall->AllocateDefaultPins();

        // GetSelf()
        UK2Node_Self* Self = CompilerContext.SpawnIntermediateNode<UK2Node_Self>(SourceNode, SourceGraph);
        Self->AllocateDefaultPins();

        UEdGraphPin* SelfOutPin = Self->FindPin(UEdGraphSchema_K2::PN_Self);
        UEdGraphPin* ViewInPin = GetViewModelCall->FindPin(ViewPinName);
        Schema->TryCreateConnection(SelfOutPin, ViewInPin);

        UEdGraphPin* ViewModelOutPin = GetViewModelCall->GetReturnValuePin();
        Schema->TryCreateConnection(ViewModelOutPin, GetSetViewModelPropertyValueInPin);
    }

    ConnectOutputPins(GetSetViewModelPropertyValueCall, CompilerContext, SourceNode, SourceGraph, Schema, ValuePin, ViewModelPropertyName);
}

void FViewModelPropertyNodeHelper::AddInputViewModelPin(UEdGraphNode& Node, UClass* ViewModelClass, bool bShowPin)
{
    // create input pin for ViewModel parameter
    FEdGraphPinType InputPinType;
    InputPinType.PinCategory = UEdGraphSchema_K2::PC_Object;
    InputPinType.PinSubCategoryObject = ViewModelClass;
    InputPinType.bIsConst = true;

    UEdGraphPin* NewPin = Node.CreatePin(EGPD_Input, InputPinType, ViewModelPinName);
    NewPin->SafeSetHidden(!bShowPin);
}

bool FViewModelPropertyNodeHelper::IsBlueprintViewModelCompatible(const UEdGraphNode* Node, UClass* ViewModelClass)
{
    UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForNode(Node);
    if (Blueprint == nullptr)
    {
        // not compatible, because not inside a blueprint
        return false;
    }

    UClass* BlueprintViewModelClass = UnrealMvvm_Impl::FViewRegistry::GetViewModelClass(Blueprint->GeneratedClass);
    if (BlueprintViewModelClass == nullptr)
    {
        // not compatible, because there is no ViewModel for current Blueprint class
        return false;
    }

    // compatible, if Blueprint has ViewModel of derived class
    return BlueprintViewModelClass->IsChildOf(ViewModelClass);
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
