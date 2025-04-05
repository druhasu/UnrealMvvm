// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "ViewModelClassSelectorHelper.h"
#include "TestBaseViewModel.h"
#include "DerivedViewModel.h"
#include "Mvvm/Impl/BaseView/ViewRegistry.h"
#include "Blueprint/UserWidget.h"
#include "Kismet2/KismetEditorUtilities.h"

BEGIN_DEFINE_SPEC(FViewModelClassSelectorSpec, "UnrealMvvm.ViewModelClassSelector", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
UBlueprint* CreateTempBlueprint(UClass* ParentClass) const;
END_DEFINE_SPEC(FViewModelClassSelectorSpec)

void FViewModelClassSelectorSpec::Define()
{
    using namespace UnrealMvvm_Impl;

    It("Should set ViewModel class to Blueprint", [this]
    {
        UBlueprint* Blueprint = CreateTempBlueprint(UUserWidget::StaticClass());

        FViewModelClassSelectorHelper::SetViewModelClass(Blueprint, UTestBaseViewModel::StaticClass());

        TestEqual("ViewModel class in registry", FViewRegistry::GetViewModelClass(Blueprint->GeneratedClass), UTestBaseViewModel::StaticClass());
        TestEqual("ViewModel class in Blueprint", FViewModelClassSelectorHelper::GetViewModelClass(Blueprint), UTestBaseViewModel::StaticClass());
    });

    It("Should set ViewModel class to Parent Blueprint", [this]
    {
        UBlueprint* ParentBlueprint = CreateTempBlueprint(UUserWidget::StaticClass());
        UBlueprint* ChildBlueprint = CreateTempBlueprint(ParentBlueprint->GeneratedClass);

        FViewModelClassSelectorHelper::SetViewModelClass(ParentBlueprint, UTestBaseViewModel::StaticClass());

        TestEqual("ViewModel class in registry", FViewRegistry::GetViewModelClass(ChildBlueprint->GeneratedClass), UTestBaseViewModel::StaticClass());
        TestEqual("ViewModel class in Blueprint", FViewModelClassSelectorHelper::GetViewModelClass(ChildBlueprint), UTestBaseViewModel::StaticClass());
    });

    It("Should override ViewModel class in Child Blueprint", [this]
    {
        UBlueprint* ParentBlueprint = CreateTempBlueprint(UUserWidget::StaticClass());
        UBlueprint* ChildBlueprint = CreateTempBlueprint(ParentBlueprint->GeneratedClass);

        FViewModelClassSelectorHelper::SetViewModelClass(ParentBlueprint, UTestBaseViewModel::StaticClass());
        FViewModelClassSelectorHelper::SetViewModelClass(ChildBlueprint, UTestDerivedViewModel::StaticClass());

        TestEqual("ViewModel class in registry", FViewRegistry::GetViewModelClass(ChildBlueprint->GeneratedClass), UTestDerivedViewModel::StaticClass());
        TestEqual("ViewModel class in Blueprint", FViewModelClassSelectorHelper::GetViewModelClass(ChildBlueprint), UTestDerivedViewModel::StaticClass());
    });

    It("Should validate valid ViewModel class in Child Blueprint", [this]
    {
        UBlueprint* ParentBlueprint = CreateTempBlueprint(UUserWidget::StaticClass());
        UBlueprint* ChildBlueprint = CreateTempBlueprint(ParentBlueprint->GeneratedClass);

        FViewModelClassSelectorHelper::SetViewModelClass(ParentBlueprint, UTestBaseViewModel::StaticClass());
        FViewModelClassSelectorHelper::SetViewModelClass(ChildBlueprint, UTestDerivedViewModel::StaticClass());

        FText Error;
        TestTrue("Valid ViewModel class", FViewModelClassSelectorHelper::ValidateViewModelClass(ChildBlueprint, &Error));
        TestTrue("Error empty", Error.IsEmpty());
    });

    It("Should not validate invalid ViewModel class in Child Blueprint", [this]
    {
        UBlueprint* ParentBlueprint = CreateTempBlueprint(UUserWidget::StaticClass());
        UBlueprint* ChildBlueprint = CreateTempBlueprint(ParentBlueprint->GeneratedClass);

        FViewModelClassSelectorHelper::SetViewModelClass(ParentBlueprint, UTestBaseViewModel::StaticClass());
        FViewModelClassSelectorHelper::SetViewModelClass(ChildBlueprint, UTestDerivedViewModel::StaticClass());

        // change ViewModel class in Parent blueprint to something else. this makes child ViewModel class no longer valid
        FViewModelClassSelectorHelper::SetViewModelClass(ParentBlueprint, UDerivedClassViewModel::StaticClass());

        FText Error;
        TestFalse("Valid ViewModel class", FViewModelClassSelectorHelper::ValidateViewModelClass(ChildBlueprint, &Error));
        TestFalse("Error empty", Error.IsEmpty());
    });
}

UBlueprint* FViewModelClassSelectorSpec::CreateTempBlueprint(UClass* ParentClass) const
{
    UObject* Outer = GetTransientPackage();

    // CreateBlueprint requires Outer object to not contain blueprints with same names, so we generate unique names for new ones
    FName NewName = MakeUniqueObjectName(Outer, UBlueprint::StaticClass(), TEXT("TestTempBP"));

    return FKismetEditorUtilities::CreateBlueprint(ParentClass, Outer, NewName, EBlueprintType::BPTYPE_Normal);
}
