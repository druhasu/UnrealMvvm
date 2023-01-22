// Copyright Andrei Sudarikov. All Rights Reserved.

#include "K2Node_ViewModelPropertyValue.h"
#include "Mvvm/BaseView.h"
#include "Mvvm/Impl/ViewModelPropertyIterator.h"
#include "ViewModelPropertyNodeHelper.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintViewModelNodeSpawner.h"

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
    UEdGraphPin* HasValuePin = FindPin(FViewModelPropertyNodeHelper::HasValuePinName);
    FViewModelPropertyNodeHelper::SpawnReadPropertyValueNodes(ValuePin, HasValuePin, CompilerContext, this, SourceGraph, ViewModelPropertyName);
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

            for (UnrealMvvm_Impl::FViewModelPropertyIterator Iter(ViewModelClass, false); Iter; ++Iter)
            {
                UBlueprintViewModelNodeSpawner* Spawner = UBlueprintViewModelNodeSpawner::CreateForProperty(GetClass(), BlueprintClass, ViewModelClass, Iter->GetProperty()->GetName());

                ActionRegistrar.AddBlueprintAction(Filter, Spawner);
            }
        }
    }
}

void UK2Node_ViewModelPropertyValue::AllocateDefaultPins()
{
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

FText UK2Node_ViewModelPropertyValue::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
    if (CachedNodeTitle.IsOutOfDate(this))
    {
        FFormatNamedArguments Args;
        Args.Add(TEXT("PropertyName"), FText::FromName(ViewModelPropertyName));

        // FText::Format() is slow, so we cache this to save on performance
        CachedNodeTitle.SetCachedText(FText::Format(NSLOCTEXT("UnrealMvvm", "ViewModelPropertyValue_Title", "Get {PropertyName}"), Args), this);
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
        CachedNodeTooltip.SetCachedText(FText::Format(NSLOCTEXT("UnrealMvvm", "ViewModelPropertyValue_Tooltip", "Read the value of property {PropertyName}"), Args), this);
    }

    return CachedNodeTooltip;
}