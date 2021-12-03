// Copyright Andrei Sudarikov. All Rights Reserved.

#include "K2Node_ViewModelPropertyValue.h"
#include "Mvvm/BaseView.h"
#include "ViewModelPropertyNodeHelper.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintViewModelNodeSpawner.h"

#define LOCTEXT_NAMESPACE "UnrealMvvm"

bool UK2Node_ViewModelPropertyValue::Modify(bool bAlwaysMarkDirty)
{
    CachedNodeTitle.MarkDirty();
    CachedNodeTooltip.MarkDirty();

    return Super::Modify(bAlwaysMarkDirty);
}

void UK2Node_ViewModelPropertyValue::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
    Super::ExpandNode(CompilerContext, SourceGraph);

    UEdGraphPin* ValuePin = FindPin(ViewModelPropertyName);
    FViewModelPropertyNodeHelper::SpawnReadPropertyValueNodes(ValuePin, CompilerContext, this, SourceGraph, ViewModelPropertyName);
}

void UK2Node_ViewModelPropertyValue::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
    const UObject* Filter = ActionRegistrar.GetActionKeyFilter();
    const UBlueprint* Blueprint = Cast<UBlueprint>(Filter);

    if (Blueprint)
    {
        UClass* BlueprintClass = Blueprint->GeneratedClass;
        if (BlueprintClass->IsChildOf<UBaseView>())
        {
            UClass* ViewModelClass = UnrealMvvm_Impl::FViewModelRegistry::GetViewModelClass(BlueprintClass);
            const TArray<UnrealMvvm_Impl::FViewModelPropertyReflection>& Properties = UnrealMvvm_Impl::FViewModelRegistry::GetProperties(ViewModelClass);

            for (auto& Property : Properties)
            {
                UBlueprintViewModelNodeSpawner* Spawner = UBlueprintViewModelNodeSpawner::CreateForProperty(GetClass(), ViewModelClass, Property.Property->GetName());

                ActionRegistrar.AddBlueprintAction(Filter, Spawner);
            }
        }
    }
}

void UK2Node_ViewModelPropertyValue::AllocateDefaultPins()
{
    // create pin based on property type
    FEdGraphPinType PinType;
    if (FViewModelPropertyNodeHelper::FillPinType(PinType, ViewModelPropertyName, ViewModelOwnerClass))
    {
        CreatePin(EGPD_Output, PinType, ViewModelPropertyName);
    }
}

FText UK2Node_ViewModelPropertyValue::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
    if (CachedNodeTitle.IsOutOfDate(this))
    {
        FFormatNamedArguments Args;
        Args.Add(TEXT("PropertyName"), FText::FromName(ViewModelPropertyName));

        // FText::Format() is slow, so we cache this to save on performance
        CachedNodeTitle.SetCachedText(FText::Format(LOCTEXT("ViewModelPropertyValue_Title", "Get {PropertyName}"), Args), this);
    }

    return CachedNodeTitle;
}

FLinearColor UK2Node_ViewModelPropertyValue::GetNodeTitleColor() const
{
    return Super::GetNodeTitleColor();
}

FSlateIcon UK2Node_ViewModelPropertyValue::GetIconAndTint(FLinearColor& OutColor) const
{
    return Super::GetIconAndTint(OutColor);
}

FText UK2Node_ViewModelPropertyValue::GetTooltipText() const
{
    if (CachedNodeTooltip.IsOutOfDate(this))
    {
        FFormatNamedArguments Args;
        Args.Add(TEXT("PropertyName"), FText::FromName(ViewModelPropertyName));

        // FText::Format() is slow, so we cache this to save on performance
        CachedNodeTooltip.SetCachedText(FText::Format(LOCTEXT("ViewModelPropertyValue_Tooltip", "Read the value of property {PropertyName}"), Args), this);
    }

    return CachedNodeTooltip;
}

#undef LOCTEXT_NAMESPACE