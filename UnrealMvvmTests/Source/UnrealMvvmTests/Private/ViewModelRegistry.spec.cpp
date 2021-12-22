// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Misc/AutomationTest.h"

#include "DerivedViewModel.h"

BEGIN_DEFINE_SPEC(ViewModelRegistrySpec, "UnrealMvvm.ViewModelRegistry", EAutomationTestFlags::ClientContext | EAutomationTestFlags::EditorContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
END_DEFINE_SPEC(ViewModelRegistrySpec)

void ViewModelRegistrySpec::Define()
{
    It("Should Return Properties Of Base And Derived ViewModels", [this]()
    {
        TArray<const UnrealMvvm_Impl::FViewModelPropertyReflection*> Properties = UnrealMvvm_Impl::FViewModelRegistry::GetProperties<UDerivedClassViewModel>();

        TestEqual("Num properties", Properties.Num(), 2);
        TestEqual("Properties[0]", Properties[0]->Property, (const FViewModelPropertyBase*)UBaseClassViewModel::BaseClassValueProperty());
        TestEqual("Properties[1]", Properties[1]->Property, (const FViewModelPropertyBase*)UDerivedClassViewModel::DerivedClassValueProperty());
    });

    It("Should Return Properties Of Base ViewModel", [this]()
    {
        TArray<const UnrealMvvm_Impl::FViewModelPropertyReflection*> Properties = UnrealMvvm_Impl::FViewModelRegistry::GetProperties<UBaseClassViewModel>();

        TestEqual("Num properties", Properties.Num(), 1);
        TestEqual("Properties[0]", Properties[0]->Property, (const FViewModelPropertyBase*)UBaseClassViewModel::BaseClassValueProperty());
    });

    It("Should Find Property Of Base ViewModel", [this]()
    {
        const UnrealMvvm_Impl::FViewModelPropertyReflection* Property = UnrealMvvm_Impl::FViewModelRegistry::FindProperty(UBaseClassViewModel::StaticClass(), TEXT("BaseClassValue"));

        TestNotNull("Property", Property);
        TestEqual("Property Pointer", Property->Property, (const FViewModelPropertyBase*)UBaseClassViewModel::BaseClassValueProperty());
    });

    It("Should Find Property Of Derived ViewModel", [this]()
    {
        const UnrealMvvm_Impl::FViewModelPropertyReflection* Property = UnrealMvvm_Impl::FViewModelRegistry::FindProperty(UDerivedClassViewModel::StaticClass(), TEXT("DerivedClassValue"));

        TestNotNull("Property", Property);
        TestEqual("Property Pointer", Property->Property, (const FViewModelPropertyBase*)UDerivedClassViewModel::DerivedClassValueProperty());
    });

    It("Should Find Property Of Base ViewModel In Derived ViewModel", [this]()
    {
        const UnrealMvvm_Impl::FViewModelPropertyReflection* Property = UnrealMvvm_Impl::FViewModelRegistry::FindProperty(UDerivedClassViewModel::StaticClass(), TEXT("BaseClassValue"));

        TestNotNull("Property", Property);
        TestEqual("Property Pointer", Property->Property, (const FViewModelPropertyBase*)UBaseClassViewModel::BaseClassValueProperty());
    });
}