// Copyright Andrei Sudarikov. All Rights Reserved.

#include "ViewModelPropertyNodeHelper.h"
#include "Mvvm/BaseViewModel.h"
#include "Mvvm/Impl/MvvmBlueprintLibrary.h"
#include "KismetCompiler.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "K2Node_CallFunction.h"
#include "K2Node_Self.h"

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
        }
    }

    return Result;
}

bool FViewModelPropertyNodeHelper::FillPinType(FEdGraphPinType& PinType, const FName& ViewModelPropertyName, UClass* ViewModelOwnerClass)
{
    const TArray<UnrealMvvm_Impl::FViewModelPropertyReflection>& Properties = UnrealMvvm_Impl::FViewModelRegistry::GetProperties(ViewModelOwnerClass);
    const UnrealMvvm_Impl::FViewModelPropertyReflection* Property = Properties.FindByPredicate([=](const UnrealMvvm_Impl::FViewModelPropertyReflection& P) { return P.Property->GetName() == ViewModelPropertyName; });

    if (Property && IsPropertyAvailableInBlueprint(*Property))
    {
        PinType.PinCategory = GetPinCategoryNameFromType(Property->PinCategoryType);
        PinType.PinSubCategoryObject = Property->GetPinSubCategoryObject();

        PinType.bIsConst = true;
        PinType.bIsReference = Property->ContainerType != EPinContainerType::None;
        PinType.ContainerType = Property->ContainerType;

        if (PinType.ContainerType == EPinContainerType::Map)
        {
            PinType.PinValueType.TerminalCategory = GetPinCategoryNameFromType(Property->PinValueCategoryType);
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
        PIN_CASE(Float);
        PIN_CASE(Name);
        PIN_CASE(Object);
        PIN_CASE(Interface);
        PIN_CASE(SoftObject);
        PIN_CASE(String);
        PIN_CASE(Text);
        PIN_CASE(Struct);
        PIN_CASE(Enum);
    }

    #undef PIN_CASE
}

void FViewModelPropertyNodeHelper::SpawnReadPropertyValueNodes(UEdGraphPin* ValuePin, FKismetCompilerContext& CompilerContext, UEdGraphNode* SourceNode, UEdGraph* SourceGraph, const FName& ViewModelPropertyName)
{
    if (ValuePin->LinkedTo.Num() > 0)
    {
        const UEdGraphSchema_K2* Schema = CompilerContext.GetSchema();

        UK2Node_CallFunction* GetViewModelPropertyValueCall = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(SourceNode, SourceGraph);
        GetViewModelPropertyValueCall->FunctionReference.SetExternalMember(GET_MEMBER_NAME_CHECKED(UMvvmBlueprintLibrary, GetViewModelPropertyValue), UMvvmBlueprintLibrary::StaticClass());
        GetViewModelPropertyValueCall->AllocateDefaultPins();

        UK2Node_Self* Self = CompilerContext.SpawnIntermediateNode<UK2Node_Self>(SourceNode, SourceGraph);
        Self->AllocateDefaultPins();

        UEdGraphPin* SelfOutPin = Self->FindPin(UEdGraphSchema_K2::PN_Self);
        UEdGraphPin* ViewInPin = GetViewModelPropertyValueCall->FindPin(TEXT("View"));
        Schema->TryCreateConnection(SelfOutPin, ViewInPin);

        UEdGraphPin* PropertyNamePin = GetViewModelPropertyValueCall->FindPin(TEXT("PropertyName"));
        PropertyNamePin->DefaultValue = ViewModelPropertyName.ToString();

        UEdGraphPin* ValueOutPin = GetViewModelPropertyValueCall->FindPin(TEXT("Value"));
        ValueOutPin->PinType.PinCategory = ValuePin->PinType.PinCategory;
        ValueOutPin->PinType.PinValueType = ValuePin->PinType.PinValueType;
        ValueOutPin->PinType.PinSubCategory = ValuePin->PinType.PinSubCategory;
        ValueOutPin->PinType.PinSubCategoryObject = ValuePin->PinType.PinSubCategoryObject;
        ValueOutPin->PinType.ContainerType = ValuePin->PinType.ContainerType;

        CompilerContext.MovePinLinksToIntermediate(*ValuePin, *ValueOutPin);
    }
}