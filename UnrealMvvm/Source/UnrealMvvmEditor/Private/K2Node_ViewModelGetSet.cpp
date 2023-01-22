// Copyright Andrei Sudarikov. All Rights Reserved.

#include "K2Node_ViewModelGetSet.h"
#include "Mvvm/BaseViewModel.h"
#include "Mvvm/MvvmBlueprintLibrary.h"
#include "Mvvm/Impl/ViewModelRegistry.h"
#include "BaseViewBlueprintExtension.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "GraphEditorSettings.h"
#include "KismetCompiler.h" // for FKismetCompilerContext
#include "K2Node_CallFunction.h"

const FName UK2Node_ViewModelGetSet::ViewPinName{ "View" };
const FName UK2Node_ViewModelGetSet::ViewModelPinName{ "ViewModel" };
const FText UK2Node_ViewModelGetSet::GetViewModelNameText = FText::FromString("Get ViewModel");
const FText UK2Node_ViewModelGetSet::SetViewModelNameText = FText::FromString("Set ViewModel");
const FText UK2Node_ViewModelGetSet::NodeCategory = FText::FromString("View Model");

void UK2Node_ViewModelGetSet::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
    UK2Node_CallFunction* CallFunctionNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
    CallFunctionNode->FunctionReference.SetExternalMember(GetFunctionName(), UMvvmBlueprintLibrary::StaticClass());
    CallFunctionNode->AllocateDefaultPins();

    UEdGraphPin* SrcViewPin = FindPin(ViewPinName);
    UEdGraphPin* DstViewPin = CallFunctionNode->FindPin(ViewPinName);
    CompilerContext.MovePinLinksToIntermediate(*SrcViewPin, *DstViewPin);

    UEdGraphPin* ScrViewModelPin = FindPin(ViewModelPinName);
    UEdGraphPin* DstViewModelPin = CallFunctionNode->FindPin(bIsSetter ? ViewModelPinName : FName("ReturnValue"));
    CompilerContext.MovePinLinksToIntermediate(*ScrViewModelPin, *DstViewModelPin);

    if (!IsNodePure())
    {
        const UEdGraphSchema_K2* Schema = CompilerContext.GetSchema();

        CompilerContext.MovePinLinksToIntermediate(*Schema->FindExecutionPin(*this, EGPD_Input), *Schema->FindExecutionPin(*CallFunctionNode, EGPD_Input));
        CompilerContext.MovePinLinksToIntermediate(*Schema->FindExecutionPin(*this, EGPD_Output), *Schema->FindExecutionPin(*CallFunctionNode, EGPD_Output));
    }
}

void UK2Node_ViewModelGetSet::NotifyPinConnectionListChanged(UEdGraphPin* Pin)
{
    if (Pin->PinName == ViewPinName)
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
    CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Object, UUserWidget::StaticClass(), ViewPinName);

    // Create ViewModel input or output pin
    CreatePin(bIsSetter ? EGPD_Input : EGPD_Output, UEdGraphSchema_K2::PC_Wildcard, nullptr, ViewModelPinName);

    // there is no convenient Init method in K2Node, so we have to resubscribe every time pins are created
    FViewModelRegistry::ViewClassChanged.RemoveAll(this);
    FViewModelRegistry::ViewClassChanged.AddUObject(this, &ThisClass::OnViewClassChanged);
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
    // copy tooltip from existing UFunction
    UFunction* Function = UMvvmBlueprintLibrary::StaticClass()->FindFunctionByName(GetFunctionName());
    return FText::FromString(UK2Node_CallFunction::GetDefaultTooltipForFunction(Function));
}

void UK2Node_ViewModelGetSet::UpdateViewModelPinType()
{
    using namespace UnrealMvvm_Impl;

    UEdGraphPin* Pin = FindPin(ViewPinName);

    if (Pin->HasAnyConnections())
    {
        UClass* ViewModelClass = FViewModelRegistry::GetViewModelClass(GetViewClass());

        UEdGraphPin* ViewModelPin = FindPin(ViewModelPinName);
        ViewModelPin->PinType.PinCategory = UEdGraphSchema_K2::PC_Object;
        ViewModelPin->PinType.PinSubCategoryObject = ViewModelClass ? ViewModelClass : UBaseViewModel::StaticClass();
    }
    else
    {
        UEdGraphPin* ViewModelPin = FindPin(ViewModelPinName);
        ViewModelPin->PinType.PinCategory = UEdGraphSchema_K2::PC_Wildcard;
        ViewModelPin->PinType.PinSubCategoryObject = nullptr;
    }
}

UClass* UK2Node_ViewModelGetSet::GetViewClass() const
{
    UEdGraphPin* ViewPin = FindPin(ViewPinName);
    if (!ViewPin->HasAnyConnections())
    {
        return nullptr;
    }

    UEdGraphPin* Pin = ViewPin->LinkedTo[0];
    if (Pin->PinType.PinSubCategoryObject.IsValid())
    {
        return CastChecked<UClass>(Pin->PinType.PinSubCategoryObject);
    }
    else if (Pin->PinType.PinSubCategory == UEdGraphSchema_K2::PSC_Self)
    {
        return GetBlueprint()->GeneratedClass;
    }

    return nullptr;
}

FName UK2Node_ViewModelGetSet::GetFunctionName() const
{
    return bIsSetter ?
        GET_FUNCTION_NAME_CHECKED(UMvvmBlueprintLibrary, SetViewModel) :
        GET_FUNCTION_NAME_CHECKED(UMvvmBlueprintLibrary, GetViewModel);
}

void UK2Node_ViewModelGetSet::OnViewClassChanged(UClass* ViewClass, UClass* ViewModelClass)
{
    if (ViewClass == GetViewClass())
    {
        UpdateViewModelPinType();
    }
}