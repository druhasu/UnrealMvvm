// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Misc/AutomationTest.h"

#include "MacrosTestViewModel.h"

BEGIN_DEFINE_SPEC(PropertyMacrosSpec, "UnrealMvvm.PropertyMacros", EAutomationTestFlags::ClientContext | EAutomationTestFlags::EditorContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
END_DEFINE_SPEC(PropertyMacrosSpec)

#define TEST_BODY_COMMON(PropertyName) \
    UMacrosTestViewModel* VM = NewObject<UMacrosTestViewModel>(); \
    VM->Set##PropertyName(5); \
    TestEqual("Stored Value", VM->Get##PropertyName(), 5)

#define TEST_ACCESSORS_COMMON(PropertyName, ExpectedGetter, ExpectedSetter, ExpectedHasSetter) \
    TestEqual(#PropertyName " Expected Getter", UMacrosTestViewModel::PropertyName ## Property()->HasPublicGetter(), ExpectedGetter); \
    TestEqual(#PropertyName " Expected Setter", UMacrosTestViewModel::PropertyName ## Property()->HasPublicSetter(), ExpectedSetter); \
    TestEqual(#PropertyName " Expected HasSetter", UMacrosTestViewModel::PropertyName ## Property()->HasSetter(), ExpectedHasSetter); \

#define TEST_OVERLOAD_COMMON(Suffix, ExpectedGetter, ExpectedSetter) \
    TEST_ACCESSORS_COMMON(PropAgAs ## Suffix, ExpectedGetter, ExpectedSetter, true); \
    TEST_ACCESSORS_COMMON(PropAgMs ## Suffix, ExpectedGetter, ExpectedSetter, true); \
    TEST_ACCESSORS_COMMON(PropMgAs ## Suffix, ExpectedGetter, ExpectedSetter, true); \
    TEST_ACCESSORS_COMMON(PropMgMs ## Suffix, ExpectedGetter, ExpectedSetter, true); \
    TEST_ACCESSORS_COMMON(PropAgAsNf ## Suffix, ExpectedGetter, ExpectedSetter, true); \
    TEST_ACCESSORS_COMMON(PropAgMsNf ## Suffix, ExpectedGetter, ExpectedSetter, true); \
    TEST_ACCESSORS_COMMON(PropMgAsNf ## Suffix, ExpectedGetter, ExpectedSetter, true); \
    TEST_ACCESSORS_COMMON(PropMgMsNf ## Suffix, ExpectedGetter, ExpectedSetter, true);

#define TEST_OVERLOAD_GETTER_ONLY(Suffix, ExpectedGetter) \
    TEST_ACCESSORS_COMMON(PropMgNf ## Suffix, ExpectedGetter, false, false)

void PropertyMacrosSpec::Define()
{
    Describe("Common methods", [this]
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

        It("Should Define ManualGetter NoField Inline Method", [this]()
        {
            UMacrosTestViewModel* VM = NewObject<UMacrosTestViewModel>();
            TestEqual("Stored Value", VM->GetPropMgNfRet(), 42);
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
    });

    Describe("Macro overloads", [this]
    {
        It("Should Define Common with Specified Getter and Setter (public, public)", [this]
        {
            TEST_OVERLOAD_COMMON(PubPub, true, true);
        });

        It("Should Define Common with Specified Getter and Setter (public, private)", [this]
        {
            TEST_OVERLOAD_COMMON(PubPriv, true, false);
        });

        It("Should Define Common with Specified Getter and Setter (private, public)", [this]
        {
            TEST_OVERLOAD_COMMON(PrivPub, false, true);
        });

        It("Should Define Common with Specified Getter and Setter (private, private)", [this]
        {
            TEST_OVERLOAD_COMMON(PrivPriv, false, false);
        });


        It("Should Define Common with Specified Setter and Default Getter (public, public)", [this]
        {
            TEST_OVERLOAD_COMMON(Pub, true, true);
        });

        It("Should Define Common with Specified Setter and Default Getter (public, private)", [this]
        {
            TEST_OVERLOAD_COMMON(Priv, true, false);
        });


        It("Should Define Common with Default Getter and Setter (public, private)", [this]
        {
            TEST_OVERLOAD_COMMON(Def, true, false);
        });


        It("Should Define Getter-Only with Specified Getter (public)", [this]
        {
            TEST_OVERLOAD_GETTER_ONLY(Pub, true);
        });

        It("Should Define Getter-Only with Specified Getter (private)", [this]
        {
            TEST_OVERLOAD_GETTER_ONLY(Priv, false);
        });


        It("Should Define Getter-Only with Default Getter (public)", [this]
        {
            TEST_OVERLOAD_GETTER_ONLY(Def, true);
        });

    });
}

#undef TEST_BODY_COMMON
#undef TEST_ACCESSORS_COMMON
#undef TEST_OVERLOAD_COMMON
#undef TEST_OVERLOAD_GETTER_ONLY