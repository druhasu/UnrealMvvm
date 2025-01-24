// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Misc/AutomationTest.h"

#include "Blueprint/UserWidget.h"

BEGIN_DEFINE_SPEC(FViewRegistrySpec, "UnrealMvvm.ViewRegistry", EAutomationTestFlags::ClientContext | EAutomationTestFlags::EditorContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
END_DEFINE_SPEC(FViewRegistrySpec)

void FViewRegistrySpec::Define()
{
    It("Should load class of a View that has invalid ViewModel class", [this]
    {
        UClass* Class = StaticLoadClass(UUserWidget::StaticClass(), nullptr, TEXT("/UnrealMvvmTests/BP_TestWidget_RemovedViewModel.BP_TestWidget_RemovedViewModel_C"));

        TestNotNull("View Class", Class);
    });
}
