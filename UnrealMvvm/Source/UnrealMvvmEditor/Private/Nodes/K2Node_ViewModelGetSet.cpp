// Copyright Andrei Sudarikov. All Rights Reserved.

#include "K2Node_ViewModelGetSet.h"
#include "Mvvm/BaseViewModel.h"
#include "Mvvm/MvvmBlueprintLibrary.h"
#include "Mvvm/Impl/BaseView/ViewRegistry.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Actor.h"
#include "ViewModelPropertyNodeHelper.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "GraphEditorSettings.h"
#include "KismetCompiler.h" // for FKismetCompilerContext
#include "K2Node_CallFunction.h"
#include "K2Node_DynamicCast.h"
#include "K2Node_Self.h"
#include "EdGraphSchema_K2.h"

const FText UK2Node_ViewModelGetSet::GetViewModelNameText = FText::FromString("Get ViewModel");
const FText UK2Node_ViewModelGetSet::SetViewModelNameText = FText::FromString("Set ViewModel");
const FText UK2Node_ViewModelGetSet::NodeCategory = FText::FromString("View Model");

void UK2Node_ViewModelGetSet::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
    UClass* ViewModelClass = UnrealMvvm_Impl::FViewRegistry::GetViewModelClass(GetViewClass());
    if (!ViewModelClass)
    {
        // do not expand if ViewModel class is unknown
        return;
    }

    // call to GetViewModel/SetViewModel function
    UK2Node_CallFunction* CallFunctionNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
    CallFunctionNode->FunctionReference.SetExternalMember(GetFunctionName(), UMvvmBlueprintLibrary::StaticClass());
    CallFunctionNode->AllocateDefaultPins();

    UEdGraphPin* SrcViewPin = FindPin(FViewModelPropertyNodeHelper::ViewPinName);
    UEdGraphPin* DstViewPin = CallFunctionNode->FindPin(FViewModelPropertyNodeHelper::ViewPinName);
    if (SrcViewPin->HasAnyConnections())
    {
        // connect View pin of this node to GetViewModel/SetViewModel function call
        CompilerContext.MovePinLinksToIntermediate(*SrcViewPin, *DstViewPin);
    }
    else
    {
        // create "Get Self" node and connect it to GetViewModel/SetViewModel function call
        UK2Node_Self* SelfNode = CompilerContext.SpawnIntermediateNode<UK2Node_Self>(this, SourceGraph);
        SelfNode->AllocateDefaultPins();
        CompilerContext.GetSchema()->TryCreateConnection(SelfNode->FindPin(UEdGraphSchema_K2::PN_Self), DstViewPin);
    }

    if (bIsSetter)
    {
        // connect ViewModel pin of this node to SetViewModel input pin
        UEdGraphPin* ScrViewModelPin = FindPin(FViewModelPropertyNodeHelper::ViewModelPinName);
        UEdGraphPin* DstViewModelPin = CallFunctionNode->FindPin(FViewModelPropertyNodeHelper::ViewModelPinName);
        CompilerContext.MovePinLinksToIntermediate(*ScrViewModelPin, *DstViewModelPin);
    }
    else
    {
        // cast to appropriate ViewModel class
        UK2Node_DynamicCast* CastNode = CompilerContext.SpawnIntermediateNode<UK2Node_DynamicCast>(this, SourceGraph);
        CastNode->TargetType = ViewModelClass;
        CastNode->SetPurity(true);
        CastNode->AllocateDefaultPins();

        // connect GetViewModel result to Cast input
        UEdGraphPin* CallFunctionResult = CallFunctionNode->FindPin(FName("ReturnValue"));
        UEdGraphPin* CastSource = CastNode->GetCastSourcePin();
        CompilerContext.GetSchema()->TryCreateConnection(CallFunctionResult, CastSource);

        // connect Cast result to output of this node
        UEdGraphPin* ScrViewModelPin = FindPin(FViewModelPropertyNodeHelper::ViewModelPinName);
        UEdGraphPin* DstViewModelPin = CastNode->GetCastResultPin();
        CompilerContext.MovePinLinksToIntermediate(*ScrViewModelPin, *DstViewModelPin);
    }

    // if node is not pure, make sure to properly connect execute pins
    if (!IsNodePure())
    {
        const UEdGraphSchema_K2* Schema = CompilerContext.GetSchema();

        CompilerContext.MovePinLinksToIntermediate(*Schema->FindExecutionPin(*this, EGPD_Input), *Schema->FindExecutionPin(*CallFunctionNode, EGPD_Input));
        CompilerContext.MovePinLinksToIntermediate(*Schema->FindExecutionPin(*this, EGPD_Output), *Schema->FindExecutionPin(*CallFunctionNode, EGPD_Output));
    }
}

void UK2Node_ViewModelGetSet::NotifyPinConnectionListChanged(UEdGraphPin* Pin)
{
    if (Pin->PinName == FViewModelPropertyNodeHelper::ViewPinName)
    {
        UpdateViewModelPinType();
    }
}

void UK2Node_ViewModelGetSet::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
    UClass* ActionKey = GetClass();

    if (ActionRegistrar.IsOpenForRegistration(ActionKey))
    {
        // add setter action
        UBlueprintNodeSpawner* SetterSpawner = UBlueprintNodeSpawner::Create<ThisClass>();
        SetterSpawner->DefaultMenuSignature.Category = NodeCategory;
        ActionRegistrar.AddBlueprintAction(SetterSpawner);

        // add getter action
        UBlueprintNodeSpawner* GetterSpawner = UBlueprintNodeSpawner::Create<ThisClass>();
        GetterSpawner->DefaultMenuSignature.Category = NodeCategory;
        GetterSpawner->CustomizeNodeDelegate.BindStatic([](UEdGraphNode* NewNode, bool bIsTemplateNode)
        {
            CastChecked<UK2Node_ViewModelGetSet>(NewNode)->bIsSetter = false;
        });
        ActionRegistrar.AddBlueprintAction(GetterSpawner);
    }
}

void UK2Node_ViewModelGetSet::PostReconstructNode()
{
    Super::PostReconstructNode();
    UpdateViewModelPinType();
}

void UK2Node_ViewModelGetSet::AllocateDefaultPins()
{
    using namespace UnrealMvvm_Impl;

    if (!IsNodePure())
    {
        // create execute pins
        CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);
        CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);
    }

    // Create View input pin
    CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Object, UObject::StaticClass(), FViewModelPropertyNodeHelper::ViewPinName);

    // Create ViewModel input or output pin
    CreatePin(bIsSetter ? EGPD_Input : EGPD_Output, UEdGraphSchema_K2::PC_Wildcard, nullptr, FViewModelPropertyNodeHelper::ViewModelPinName);

    // there is no convenient Init method in K2Node, so we have to resubscribe every time pins are created
    FViewRegistry::ViewModelClassChanged.RemoveAll(this);
    FViewRegistry::ViewModelClassChanged.AddUObject(this, &ThisClass::OnViewClassChanged);

    UpdateViewModelPinType();
}

FString UK2Node_ViewModelGetSet::GetPinMetaData(FName InPinName, FName InKey)
{
    // force our View pin to be drawn as "self" pin
    if (InPinName == FViewModelPropertyNodeHelper::ViewPinName && InKey == FBlueprintMetadata::MD_DefaultToSelf)
    {
        return FViewModelPropertyNodeHelper::ViewPinName.ToString();
    }

    return Super::GetPinMetaData(InPinName, InKey);
}

FText UK2Node_ViewModelGetSet::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
    return bIsSetter ? SetViewModelNameText : GetViewModelNameText;
}

FLinearColor UK2Node_ViewModelGetSet::GetNodeTitleColor() const
{
    return Super::GetNodeTitleColor();
}

FSlateIcon UK2Node_ViewModelGetSet::GetIconAndTint(FLinearColor& OutColor) const
{
    OutColor = IsNodePure() ?
        GetDefault<UGraphEditorSettings>()->PureFunctionCallNodeTitleColor :
        GetDefault<UGraphEditorSettings>()->FunctionCallNodeTitleColor;

    static FSlateIcon Icon(FAppStyle::GetAppStyleSetName(), "Kismet.AllClasses.FunctionIcon");
    return Icon;
}

FText UK2Node_ViewModelGetSet::GetTooltipText() const
{
    return bIsSetter ?
        INVTEXT("Sets ViewModel to a View") :
        INVTEXT("Returns ViewModel from a View");
}

void UK2Node_ViewModelGetSet::UpdateViewModelPinType()
{
    using namespace UnrealMvvm_Impl;

    UEdGraphPin* Pin = FindPin(FViewModelPropertyNodeHelper::ViewPinName);

    UClass* ViewClass = GetViewClass();

    if (ViewClass != nullptr)
    {
        UClass* ViewModelClass = FViewRegistry::GetViewModelClass(ViewClass);

        UEdGraphPin* ViewModelPin = FindPin(FViewModelPropertyNodeHelper::ViewModelPinName);
        ViewModelPin->PinType.PinCategory = UEdGraphSchema_K2::PC_Object;
        ViewModelPin->PinType.PinSubCategoryObject = ViewModelClass ? ViewModelClass : UBaseViewModel::StaticClass();
    }
    else
    {
        UEdGraphPin* ViewModelPin = FindPin(FViewModelPropertyNodeHelper::ViewModelPinName);
        ViewModelPin->PinType.PinCategory = UEdGraphSchema_K2::PC_Wildcard;
        ViewModelPin->PinType.PinSubCategoryObject = nullptr;
    }
}

UClass* UK2Node_ViewModelGetSet::GetViewClass() const
{
    UEdGraphPin* ViewPin = FindPin(FViewModelPropertyNodeHelper::ViewPinName);
    UBlueprint* Blueprint = GetBlueprint();

    if (!ViewPin->HasAnyConnections())
    {
        if (Blueprint != nullptr)
        {
            return Blueprint->GeneratedClass;
        }

        return nullptr;
    }

    UEdGraphPin* Pin = ViewPin->LinkedTo[0];
    if (Pin->PinType.PinSubCategoryObject.IsValid())
    {
        return CastChecked<UClass>(Pin->PinType.PinSubCategoryObject);
    }
    else if (Pin->PinType.PinSubCategory == UEdGraphSchema_K2::PSC_Self)
    {
        return Blueprint->GeneratedClass;
    }

    return nullptr;
}

FName UK2Node_ViewModelGetSet::GetFunctionName() const
{
    UClass* ViewClass = GetViewClass();

    if (ViewClass->IsChildOf<UUserWidget>())
    {
        return bIsSetter ?
            GET_FUNCTION_NAME_CHECKED(UMvvmBlueprintLibrary, SetViewModelToWidget) :
            GET_FUNCTION_NAME_CHECKED(UMvvmBlueprintLibrary, GetViewModelFromWidget);
    }
    else
    {
        ensureAlways(ViewClass->IsChildOf<AActor>());
        return bIsSetter ?
            GET_FUNCTION_NAME_CHECKED(UMvvmBlueprintLibrary, SetViewModelToActor) :
            GET_FUNCTION_NAME_CHECKED(UMvvmBlueprintLibrary, GetViewModelFromActor);
    }
}

void UK2Node_ViewModelGetSet::OnViewClassChanged(UClass* ViewClass, UClass* ViewModelClass)
{
    if (ViewClass == GetViewClass())
    {
        UpdateViewModelPinType();
    }
}
