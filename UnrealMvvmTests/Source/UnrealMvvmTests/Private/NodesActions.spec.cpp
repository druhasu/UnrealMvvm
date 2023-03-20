// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "BlueprintActionDatabase.h"
#include "BlueprintNodeSpawner.h"

BEGIN_DEFINE_SPEC(FNodesActionsSpec, "UnrealMvvm.Nodes Actions", EAutomationTestFlags::ClientContext | EAutomationTestFlags::EditorContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
bool HasSpawner(const FName& ActionObjectName, const FName& SpawnerNodeClassName) const;
END_DEFINE_SPEC(FNodesActionsSpec)

void FNodesActionsSpec::Define()
{
    BeforeEach([]
    {
        // load Blueprint class, so node actions are added to database
        StaticLoadClass(UObject::StaticClass(), nullptr, TEXT("/UnrealMvvmTests/BP_TestBaseView.BP_TestBaseView_C"));
    });

    It("Should provide GetPropertyValue node", [this]
    {
        const bool bFoundSpawner = HasSpawner("BP_TestBaseView", "K2Node_ViewModelPropertyValue");
        TestTrue("Found K2Node_ViewModelPropertyValue Spawner", bFoundSpawner);
    });

    It("Should provide ViewModelGetSet node", [this]
    {
        const bool bFoundSpawner = HasSpawner("K2Node_ViewModelGetSet", "K2Node_ViewModelGetSet");
        TestTrue("Found K2Node_ViewModelPropertyValue Spawner", bFoundSpawner);
    });

    It("Should provide ViewModelChanged node", [this]
    {
        const bool bFoundSpawner = HasSpawner("K2Node_ViewModelChanged", "K2Node_ViewModelChanged");
        TestTrue("Found K2Node_ViewModelPropertyValue Spawner", bFoundSpawner);
    });
}

bool FNodesActionsSpec::HasSpawner(const FName& ActionObjectName, const FName& SpawnerNodeClassName) const
{
    const FBlueprintActionDatabase::FActionRegistry& ActionRegistry = FBlueprintActionDatabase::Get().GetAllActions();
    
    for (auto& Pair : ActionRegistry)
    {
        if (UObject* ActionObject = Pair.Key.ResolveObjectPtr(); ActionObject != nullptr && ActionObject->GetFName() == ActionObjectName)
        {
            return Pair.Value.ContainsByPredicate([&](const UBlueprintNodeSpawner* Spawner) { return Spawner->NodeClass->GetFName() == SpawnerNodeClassName; });
        }
    }

    return false;
}