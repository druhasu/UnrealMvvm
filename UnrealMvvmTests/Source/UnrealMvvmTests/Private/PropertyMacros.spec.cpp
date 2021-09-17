// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Misc/AutomationTest.h"

#include "MacrosTestViewModel.h"

BEGIN_DEFINE_SPEC(PropertyMacrosSpec, "UnrealMvvm.PropertyMacros", EAutomationTestFlags::ClientContext | EAutomationTestFlags::EditorContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
END_DEFINE_SPEC(PropertyMacrosSpec)

#define TEST_BODY_COMMON(PropertyName) \
    UMacrosTestViewModel* VM = NewObject<UMacrosTestViewModel>(); \
    VM->Set##PropertyName(5); \
    TestEqual("Stored Value", VM->Get##PropertyName(), 5)

void PropertyMacrosSpec::Define()
{
    It("Should Define AutoGetter AutoSetter", [this]()
    {
        TEST_BODY_COMMON(PropAgAs);
    });

    It("Should Define AutoGetter ManualSetter", [this]()
    {
        TEST_BODY_COMMON(PropAgMs);
    });

    It("Should Define ManualGetter AutoSetter", [this]()
    {
        TEST_BODY_COMMON(PropMgAs);
    });

    It("Should Define ManualGetter ManualSetter", [this]()
    {
        TEST_BODY_COMMON(PropMgMs);
    });


    It("Should Define AutoGetter AutoSetter NoField", [this]()
    {
        TEST_BODY_COMMON(PropAgAsNf);
    });

    It("Should Define AutoGetter ManualSetter NoField", [this]()
    {
        TEST_BODY_COMMON(PropAgMsNf);
    });

    It("Should Define ManualGetter AutoSetter NoField", [this]()
    {
        TEST_BODY_COMMON(PropMgAsNf);
    });

    It("Should Define ManualGetter ManualSetter NoField", [this]()
    {
        TEST_BODY_COMMON(PropMgMsNf);
    });


    It("Should Define ManualGetter NoField", [this]()
    {
        UMacrosTestViewModel* VM = NewObject<UMacrosTestViewModel>();
        TestEqual("Stored Value", VM->GetPropMgNf(), 42);
    });


    It("Should Define Reference", [this]()
    {
        UMacrosTestViewModel* VM = NewObject<UMacrosTestViewModel>();
        VM->SetRefPropAgAs(5);
        const int32& RefValue = VM->GetRefPropAgAs();
        TestEqual("Stored Value", RefValue, 5);
    });


    It("Should Define Pointer", [this]()
    {
        UMacrosTestViewModel* VM = NewObject<UMacrosTestViewModel>();
        int32 Value = 5;
        VM->SetPtrPropAgAs(&Value);
        TestEqual("Stored Value", VM->GetPtrPropAgAs(), &Value);
    });
}