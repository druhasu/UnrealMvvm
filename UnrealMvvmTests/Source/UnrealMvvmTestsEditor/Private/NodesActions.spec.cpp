// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UnrealMvvmEditor/Private/Nodes/K2Node_ViewModelPropertyBase.h"
#include "TestBaseViewModel.h"
#include "DerivedViewModel.h"
#include "BlueprintActionDatabase.h"
#include "BlueprintNodeSpawner.h"

BEGIN_DEFINE_SPEC(FNodesActionsSpec, "UnrealMvvm.Nodes Actions", EAutomationTestFlags::ClientContext | EAutomationTestFlags::EditorContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
const UBlueprintNodeSpawner* FindSpawner(const FName& SpawnerNodeClassName) const;
const UBlueprintNodeSpawner* FindSpawner(const FName& SpawnerNodeClassName, UClass* ViewModelClass, FName ViewModelProperty) const;
const UBlueprintNodeSpawner* FindSpawner(const FName& SpawnerNodeClassName, TFunction<bool(const UBlueprintNodeSpawner*)> Predicate) const;
END_DEFINE_SPEC(FNodesActionsSpec)

void FNodesActionsSpec::Define()
{
    BeforeEach([]
    {
        // load Blueprint class, so node actions are added to database
        StaticLoadClass(UObject::StaticClass(), nullptr, TEXT("/UnrealMvvmTests/WidgetView/BP_TestWidgetView_BlueprintBased.BP_TestWidgetView_BlueprintBased_C"));
    });

    It("Should provide GetPropertyValue node for public property", [this]
    {
        auto* FoundSpawner = FindSpawner("K2Node_GetViewModelPropertyValue", UTestBaseViewModel::StaticClass(), UTestBaseViewModel::IntValueProperty()->GetName());
        TestNotNull("Found K2Node_GetViewModelPropertyValue Spawner", FoundSpawner);
    });

    It("Should not provide GetPropertyValue node for private property", [this]
    {
        auto* FoundSpawner = FindSpawner("K2Node_GetViewModelPropertyValue", UTestBaseViewModel::StaticClass(), UTestBaseViewModel::NoPublicGetterProperty()->GetName());
        TestNull("Found K2Node_GetViewModelPropertyValue Spawner", FoundSpawner);
    });

    It("Should provide SetPropertyValue node for public property", [this]
    {
        auto* FoundSpawner = FindSpawner("K2Node_SetViewModelPropertyValue", UTestBaseViewModel::StaticClass(), UTestBaseViewModel::IntValueProperty()->GetName());
        TestNotNull("Found K2Node_SetViewModelPropertyValue Spawner", FoundSpawner);
    });

    It("Should not provide SetPropertyValue node for private property", [this]
    {
        auto* FoundSpawner = FindSpawner("K2Node_SetViewModelPropertyValue", UTestBaseViewModel::StaticClass(), UTestBaseViewModel::NoPublicSetterProperty()->GetName());
        TestNull("Found K2Node_SetViewModelPropertyValue Spawner", FoundSpawner);
    });

    It("Should provide ViewModelGetSet node", [this]
    {
        auto* FoundSpawner = FindSpawner("K2Node_ViewModelGetSet");
        TestNotNull("Found K2Node_ViewModelGetSet Spawner", FoundSpawner);
    });

    It("Should provide ViewModelChanged node", [this]
    {
        auto* FoundSpawner = FindSpawner("K2Node_ViewModelChanged");
        TestNotNull("Found K2Node_ViewModelChanged Spawner", FoundSpawner);
    });

    It("Should provide GetPropertyValue node for public property of Derived ViewModel", [this]
    {
        auto* FoundSpawner = FindSpawner("K2Node_GetViewModelPropertyValue", UDerivedClassViewModel::StaticClass(), UDerivedClassViewModel::DerivedClassValueProperty()->GetName());
        TestNotNull("Found K2Node_GetViewModelPropertyValue Spawner", FoundSpawner);

        UBlueprint* Blueprint = Cast<UBlueprint>(StaticLoadObject(UBlueprint::StaticClass(), nullptr, TEXT("/UnrealMvvmTests/WidgetView/BP_TestWidgetView_DerivedViewModel.BP_TestWidgetView_DerivedViewModel")));
        FBlueprintActionFilter Filter;
        Filter.Context.Blueprints.Add(Blueprint);

        const bool bFiltered = FoundSpawner->IsTemplateNodeFilteredOut(Filter);
        TestFalse("Node is Filtered out", bFiltered);
    });

    It("Should provide GetPropertyValue node for public property of Base ViewModel", [this]
    {
        auto* FoundSpawner = FindSpawner("K2Node_GetViewModelPropertyValue", UBaseClassViewModel::StaticClass(), UBaseClassViewModel::BaseClassValueProperty()->GetName());
        TestNotNull("Found K2Node_GetViewModelPropertyValue Spawner", FoundSpawner);

        UBlueprint* Blueprint = Cast<UBlueprint>(StaticLoadObject(UBlueprint::StaticClass(), nullptr, TEXT("/UnrealMvvmTests/WidgetView/BP_TestWidgetView_DerivedViewModel.BP_TestWidgetView_DerivedViewModel")));
        FBlueprintActionFilter Filter;
        Filter.Context.Blueprints.Add(Blueprint);

        const bool bFiltered = FoundSpawner->IsTemplateNodeFilteredOut(Filter);
        TestFalse("Node is Filtered out", bFiltered);
    });
}

const UBlueprintNodeSpawner* FNodesActionsSpec::FindSpawner(const FName& SpawnerNodeClassName) const
{
    return FindSpawner(SpawnerNodeClassName, [&](const UBlueprintNodeSpawner* Spawner)
    {
        return Spawner->NodeClass->GetFName() == SpawnerNodeClassName;
    });
}

const UBlueprintNodeSpawner* FNodesActionsSpec::FindSpawner(const FName& SpawnerNodeClassName, UClass* ViewModelClass, FName ViewModelProperty) const
{
    return FindSpawner(SpawnerNodeClassName, [&](const UBlueprintNodeSpawner* Spawner)
    {
        if (auto* Node = Cast<UK2Node_ViewModelPropertyBase>(Spawner->GetTemplateNode()))
        {
            return Node->ViewModelOwnerClass == ViewModelClass && Node->ViewModelPropertyName == ViewModelProperty;
        }

        return false;
    });
}

const UBlueprintNodeSpawner* FNodesActionsSpec::FindSpawner(const FName& SpawnerNodeClassName, TFunction<bool(const UBlueprintNodeSpawner*)> Predicate) const
{
    const FBlueprintActionDatabase::FActionRegistry& ActionRegistry = FBlueprintActionDatabase::Get().GetAllActions();

    for (auto& Pair : ActionRegistry)
    {
        if (UObject* ActionObject = Pair.Key.ResolveObjectPtr(); ActionObject != nullptr && ActionObject->GetFName() == SpawnerNodeClassName)
        {
            for (const UBlueprintNodeSpawner* Spawner : Pair.Value)
            {
                if (Predicate(Spawner))
                {
                    return Spawner;
                }
            }
        }
    }

    return nullptr;
}
