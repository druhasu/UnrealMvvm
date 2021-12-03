// Copyright Andrei Sudarikov. All Rights Reserved.

#include "BlueprintViewModelNodeSpawner.h"
#include "K2Node_ViewModelPropertyValue.h"
#include "EditorCategoryUtils.h"

#define LOCTEXT_NAMESPACE "UnrealMvvm"

UBlueprintViewModelNodeSpawner* UBlueprintViewModelNodeSpawner::CreateForProperty(TSubclassOf<UK2Node> NodeClass, UClass* ViewModelClass, const FName& ViewModelPropertyName)
{
    UBlueprintViewModelNodeSpawner* Spawner = NewObject<UBlueprintViewModelNodeSpawner>();
    Spawner->ViewModelClass = ViewModelClass;
    Spawner->ViewModelPropertyName = ViewModelPropertyName;
    Spawner->NodeClass = NodeClass;

    Spawner->DefaultMenuSignature.MenuName = FText::Format(LOCTEXT("Action.GetterMenuName", "Get {0}"), FText::FromName(ViewModelPropertyName));
    Spawner->DefaultMenuSignature.Tooltip = FText::Format(LOCTEXT("Action.GetterMenuTooltip", "Read the value of property {0}"), FText::FromName(ViewModelPropertyName));
    Spawner->DefaultMenuSignature.Category = FEditorCategoryUtils::BuildCategoryString(FCommonEditorCategory::Variables, ViewModelClass->GetDisplayNameText());

    return Spawner;
}

FBlueprintNodeSignature UBlueprintViewModelNodeSpawner::GetSpawnerSignature() const
{
    FBlueprintNodeSignature Result(NodeClass);
    Result.AddKeyValue(ViewModelPropertyName.ToString());
    return Result;
}

UEdGraphNode* UBlueprintViewModelNodeSpawner::Invoke(UEdGraph* ParentGraph, FBindingSet const& Bindings, FVector2D const Location) const
{
    auto PostSpawnSetupLambda = [](UEdGraphNode* NewNode, bool bIsTemplateNode, UClass* ViewModelClass, FName ViewModelPropertyName)
    {
        UK2Node_ViewModelPropertyValue* Node = CastChecked<UK2Node_ViewModelPropertyValue>(NewNode);
        Node->ViewModelOwnerClass = ViewModelClass;
        Node->ViewModelPropertyName = ViewModelPropertyName;
    };

    FCustomizeNodeDelegate PostSpawnSetupDelegate = FCustomizeNodeDelegate::CreateStatic(PostSpawnSetupLambda, ViewModelClass, ViewModelPropertyName);
    return Super::SpawnNode<UEdGraphNode>(NodeClass, ParentGraph, Bindings, Location, PostSpawnSetupDelegate);
}

FBlueprintActionUiSpec UBlueprintViewModelNodeSpawner::GetUiSpec(FBlueprintActionContext const& Context, FBindingSet const& Bindings) const
{
    return Super::GetUiSpec(Context, Bindings);
}