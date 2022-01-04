// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Misc/AutomationTest.h"

#include "DerivedViewModel.h"
#include "PinTraitsViewModel.h"

using namespace UnrealMvvm_Impl;

BEGIN_DEFINE_SPEC(ViewModelRegistrySpec, "UnrealMvvm.ViewModelRegistry", EAutomationTestFlags::ClientContext | EAutomationTestFlags::EditorContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
END_DEFINE_SPEC(ViewModelRegistrySpec)

void ViewModelRegistrySpec::Define()
{
    Describe("GetProperties", [this]()
    {
        It("Should Return Properties Of Base And Derived ViewModels", [this]()
        {
            TArray<const FViewModelPropertyReflection*> Properties = FViewModelRegistry::GetProperties<UDerivedClassViewModel>();

            TestEqual("Num properties", Properties.Num(), 2);
            TestEqual("Properties[0]", Properties[0]->Property, (const FViewModelPropertyBase*)UBaseClassViewModel::BaseClassValueProperty());
            TestEqual("Properties[1]", Properties[1]->Property, (const FViewModelPropertyBase*)UDerivedClassViewModel::DerivedClassValueProperty());
        });

        It("Should Return Properties Of Base ViewModel", [this]()
        {
            TArray<const FViewModelPropertyReflection*> Properties = FViewModelRegistry::GetProperties<UBaseClassViewModel>();

            TestEqual("Num properties", Properties.Num(), 1);
            TestEqual("Properties[0]", Properties[0]->Property, (const FViewModelPropertyBase*)UBaseClassViewModel::BaseClassValueProperty());
        });
    });

    Describe("FindProperty", [this]()
    {
        It("Should Find Property Of Base ViewModel", [this]()
        {
            const FViewModelPropertyReflection* Property = FViewModelRegistry::FindProperty<UBaseClassViewModel>(TEXT("BaseClassValue"));

            TestNotNull("Property", Property);
            TestEqual("Property Pointer", Property->Property, (const FViewModelPropertyBase*)UBaseClassViewModel::BaseClassValueProperty());
        });

        It("Should Find Property Of Derived ViewModel", [this]()
        {
            const FViewModelPropertyReflection* Property = FViewModelRegistry::FindProperty<UDerivedClassViewModel>(TEXT("DerivedClassValue"));

            TestNotNull("Property", Property);
            TestEqual("Property Pointer", Property->Property, (const FViewModelPropertyBase*)UDerivedClassViewModel::DerivedClassValueProperty());
        });

        It("Should Find Property Of Base ViewModel In Derived ViewModel", [this]()
        {
            const FViewModelPropertyReflection* Property = FViewModelRegistry::FindProperty<UDerivedClassViewModel>(TEXT("BaseClassValue"));

            TestNotNull("Property", Property);
            TestEqual("Property Pointer", Property->Property, (const FViewModelPropertyBase*)UBaseClassViewModel::BaseClassValueProperty());
        });
    });

    Describe("CopyValueToMemory", [this]()
    {
        Describe("Regular", [this]()
        {
            It("Should Copy int32", [this]()
            {
                UPinTraitsViewModel* ViewModel = NewObject<UPinTraitsViewModel>();
                const FViewModelPropertyReflection* Property = FViewModelRegistry::FindProperty<UPinTraitsViewModel>(TEXT("MyInt"));

                int32 OutValue = 0;
                bool HasValue = false;

                ViewModel->SetMyInt(123);
                Property->CopyValueToMemory(ViewModel, &OutValue, HasValue);

                TestEqual("OutValue", OutValue, 123);
                TestTrue("HasValue", HasValue);
            });

            It("Should Copy UObject*", [this]()
            {
                UPinTraitsViewModel* ViewModel = NewObject<UPinTraitsViewModel>();
                const FViewModelPropertyReflection* Property = FViewModelRegistry::FindProperty<UPinTraitsViewModel>(TEXT("MyObject"));

                UObject* OutValue = 0;
                bool HasValue = false;

                ViewModel->SetMyObject(ViewModel);
                Property->CopyValueToMemory(ViewModel, &OutValue, HasValue);

                TestEqual("OutValue", OutValue, (UObject*)ViewModel);
                TestTrue("HasValue", HasValue);
            });

            It("Should Copy TArray", [this]()
            {
                UPinTraitsViewModel* ViewModel = NewObject<UPinTraitsViewModel>();
                const FViewModelPropertyReflection* Property = FViewModelRegistry::FindProperty<UPinTraitsViewModel>(TEXT("MyIntArray"));

                TArray<int32> OutValue;
                bool HasValue = false;

                ViewModel->SetMyIntArray({ 123, 321 });
                Property->CopyValueToMemory(ViewModel, &OutValue, HasValue);

                TestEqual("OutValue.Num", OutValue.Num(), 2);
                TestEqual("OutValue[0]", OutValue[0], 123);
                TestEqual("OutValue[1]", OutValue[1], 321);
                TestTrue("HasValue", HasValue);
            });
        });

        Describe("Optional", [this]()
        {
            It("Should Copy Optional int32", [this]()
            {
                UPinTraitsViewModel* ViewModel = NewObject<UPinTraitsViewModel>();
                const FViewModelPropertyReflection* Property = FViewModelRegistry::FindProperty<UPinTraitsViewModel>(TEXT("MyIntOptional"));

                int32 OutValue = 0;
                bool HasValue = false;

                Property->CopyValueToMemory(ViewModel, &OutValue, HasValue);
                TestFalse("HasValue", HasValue);

                ViewModel->SetMyIntOptional(123);
                Property->CopyValueToMemory(ViewModel, &OutValue, HasValue);

                TestEqual("OutValue", OutValue, 123);
                TestTrue("HasValue", HasValue);
            });

            It("Should Copy Optional UObject*", [this]()
            {
                UPinTraitsViewModel* ViewModel = NewObject<UPinTraitsViewModel>();
                const FViewModelPropertyReflection* Property = FViewModelRegistry::FindProperty<UPinTraitsViewModel>(TEXT("MyObjectOptional"));

                UObject* OutValue = 0;
                bool HasValue = false;

                Property->CopyValueToMemory(ViewModel, &OutValue, HasValue);
                TestFalse("HasValue", HasValue);

                ViewModel->SetMyObjectOptional(ViewModel);
                Property->CopyValueToMemory(ViewModel, &OutValue, HasValue);

                TestEqual("OutValue", OutValue, (UObject*)ViewModel);
                TestTrue("HasValue", HasValue);
            });

            It("Should Copy Optional TArray", [this]()
            {
                UPinTraitsViewModel* ViewModel = NewObject<UPinTraitsViewModel>();
                const FViewModelPropertyReflection* Property = FViewModelRegistry::FindProperty<UPinTraitsViewModel>(TEXT("MyIntArrayOptional"));

                TArray<int32> OutValue;
                bool HasValue = false;

                Property->CopyValueToMemory(ViewModel, &OutValue, HasValue);
                TestFalse("HasValue", HasValue);

                ViewModel->SetMyIntArrayOptional(TArray<int32>({ 123, 321 }));
                Property->CopyValueToMemory(ViewModel, &OutValue, HasValue);

                TestEqual("OutValue.Num", OutValue.Num(), 2);
                TestEqual("OutValue[0]", OutValue[0], 123);
                TestEqual("OutValue[1]", OutValue[1], 321);
                TestTrue("HasValue", HasValue);
            });
        });
    });
}