// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UnrealMvvmEditor/Private/K2Node_ViewModelPropertyBase.h"
#include "TestBaseViewModel.h"
#include "BlueprintActionDatabase.h"
#include "BlueprintNodeSpawner.h"

BEGIN_DEFINE_SPEC(FNodesActionsSpec, "UnrealMvvm.Nodes Actions", EAutomationTestFlags::ClientContext | EAutomationTestFlags::EditorContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
bool HasSpawner(const FName& SpawnerNodeClassName) const;
bool HasSpawner(const FName& SpawnerNodeClassName, UClass* ViewModelClass, FName ViewModelProperty) const;
END_DEFINE_SPEC(FNodesActionsSpec)

void FNodesActionsSpec::Define()
{
    BeforeEach([]
    {
        // load Blueprint class, so node actions are added to database
        StaticLoadClass(UObject::StaticClass(), nullptr, TEXT("/UnrealMvvmTests/BP_TestBaseView.BP_TestBaseView_C"));
    });

    It("Should provide GetPropertyValue node for public property", [this]
    {
        const bool bFoundSpawner = HasSpawner("K2Node_GetViewModelPropertyValue", UTestBaseViewModel::StaticClass(), UTestBaseViewModel::IntValueProperty()->GetName());
        TestTrue("Found K2Node_GetViewModelPropertyValue Spawner", bFoundSpawner);
    });

    It("Should not provide GetPropertyValue node for private property", [this]
    {
        const bool bFoundSpawner = HasSpawner("K2Node_GetViewModelPropertyValue", UTestBaseViewModel::StaticClass(), UTestBaseViewModel::NoPublicGetterProperty()->GetName());
        TestFalse("Found K2Node_GetViewModelPropertyValue Spawner", bFoundSpawner);
    });

    It("Should provide SetPropertyValue node for public property", [this]
    {
        const bool bFoundSpawner = HasSpawner("K2Node_SetViewModelPropertyValue", UTestBaseViewModel::StaticClass(), UTestBaseViewModel::IntValueProperty()->GetName());
        TestTrue("Found K2Node_SetViewModelPropertyValue Spawner", bFoundSpawner);
    });

    It("Should not provide SetPropertyValue node for private property", [this]
    {
        const bool bFoundSpawner = HasSpawner("K2Node_SetViewModelPropertyValue", UTestBaseViewModel::StaticClass(), UTestBaseViewModel::NoPublicSetterProperty()->GetName());
        TestFalse("Found K2Node_SetViewModelPropertyValue Spawner", bFoundSpawner);
    });

    It("Should provide ViewModelGetSet node", [this]
    {
        const bool bFoundSpawner = HasSpawner("K2Node_ViewModelGetSet");
        TestTrue("Found K2Node_ViewModelGetSet Spawner", bFoundSpawner);
    });

    It("Should provide ViewModelChanged node", [this]
    {
        const bool bFoundSpawner = HasSpawner("K2Node_ViewModelChanged");
        TestTrue("Found K2Node_ViewModelChanged Spawner", bFoundSpawner);
    });
}

bool FNodesActionsSpec::HasSpawner(const FName& SpawnerNodeClassName) const
{
    const FBlueprintActionDatabase::FActionRegistry& ActionRegistry = FBlueprintActionDatabase::Get().GetAllActions();
    
    for (auto& Pair : ActionRegistry)
    {
        if (UObject* ActionObject = Pair.Key.ResolveObjectPtr(); ActionObject != nullptr && ActionObject->GetFName() == SpawnerNodeClassName)
        {
            return Pair.Value.ContainsByPredicate([&](const UBlueprintNodeSpawner* Spawner) { return Spawner->NodeClass->GetFName() == SpawnerNodeClassName; });
        }
    }

    return false;
}

bool FNodesActionsSpec::HasSpawner(const FName& SpawnerNodeClassName, UClass* ViewModelClass, FName ViewModelProperty) const
{
    const FBlueprintActionDatabase::FActionRegistry& ActionRegistry = FBlueprintActionDatabase::Get().GetAllActions();

    for (auto& Pair : ActionRegistry)
    {
        if (UObject* ActionObject = Pair.Key.ResolveObjectPtr(); ActionObject != nullptr && ActionObject->GetFName() == SpawnerNodeClassName)
        {
            for (const UBlueprintNodeSpawner* Spawner : Pair.Value)
            {
                if (auto* Node = Cast<UK2Node_ViewModelPropertyBase>(Spawner->GetTemplateNode()))
                {
                    if (Node->ViewModelOwnerClass == ViewModelClass && Node->ViewModelPropertyName == ViewModelProperty)
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}