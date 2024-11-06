// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Misc/AutomationTest.h"

#include "Mvvm/Impl/Binding/BindingConfigurationBuilder.h"
#include "BindingWorkerTestViewModel.h"

BEGIN_DEFINE_SPEC(FBindingConfigurationBuilderSpec, "UnrealMvvm.BindingConfigurationBuilder", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter)
void TestViewModel(const FString& Prefix, const UnrealMvvm_Impl::FResolvedViewModelEntry& Actual, const UnrealMvvm_Impl::FResolvedViewModelEntry& Expected);
void TestProperty(const FString& Prefix, const UnrealMvvm_Impl::FResolvedPropertyEntry& Actual, const UnrealMvvm_Impl::FResolvedPropertyEntry& Expected);
END_DEFINE_SPEC(FBindingConfigurationBuilderSpec)

void FBindingConfigurationBuilderSpec::Define()
{
    using namespace UnrealMvvm_Impl;

    Describe("Native", [this]
    {
        It("Should resolve bindings for single property", [this]
        {
            FBindingConfigurationBuilder Builder(UBindingWorkerViewModel_Root::StaticClass());

            Builder.AddBinding({ UBindingWorkerViewModel_Root::IntValueProperty() });

            FBindingConfiguration Configuration = Builder.Build();

            TArrayView<FResolvedViewModelEntry> ViewModels = Configuration.GetViewModels();
            TArrayView<FResolvedPropertyEntry> Properties = Configuration.GetProperties();

            TestEqual("Num ViewModels", ViewModels.Num(), 1);
            TestEqual("Num Properties", Properties.Num(), 1);

            TestViewModel("ViewModel[0]", ViewModels[0], { { UBindingWorkerViewModel_Root::StaticClass() }, 0, 1 });
            TestProperty("Property[0]", Properties[0], { UBindingWorkerViewModel_Root::IntValueProperty(), INDEX_NONE });
        });

        It("Should resolve bindings for multiple properties", [this]
        {
            FBindingConfigurationBuilder Builder(UBindingWorkerViewModel_Root::StaticClass());

            Builder.AddBinding({ UBindingWorkerViewModel_Root::IntValueProperty() });
            Builder.AddBinding({ UBindingWorkerViewModel_Root::MinIntValueProperty() });
            Builder.AddBinding({ UBindingWorkerViewModel_Root::MaxIntValueProperty() });

            FBindingConfiguration Configuration = Builder.Build();

            TArrayView<FResolvedViewModelEntry> ViewModels = Configuration.GetViewModels();
            TArrayView<FResolvedPropertyEntry> Properties = Configuration.GetProperties();

            TestEqual("Num ViewModels", ViewModels.Num(), 1);
            TestEqual("Num Properties", Properties.Num(), 3);

            TestViewModel("ViewModel[0]", ViewModels[0], { { UBindingWorkerViewModel_Root::StaticClass() }, 0, 3 });

            TestProperty("Property[0]", Properties[0], { UBindingWorkerViewModel_Root::IntValueProperty(), INDEX_NONE });
            TestProperty("Property[1]", Properties[1], { UBindingWorkerViewModel_Root::MinIntValueProperty(), INDEX_NONE });
            TestProperty("Property[2]", Properties[2], { UBindingWorkerViewModel_Root::MaxIntValueProperty(), INDEX_NONE });
        });

        It("Should resolve bindings for single property by path", [this]
        {
            FBindingConfigurationBuilder Builder(UBindingWorkerViewModel_Root::StaticClass());

            Builder.AddBinding({ UBindingWorkerViewModel_Root::ChildProperty(), UBindingWorkerViewModel_FirstChild::ChildProperty(), UBindingWorkerViewModel_SecondChild::IntValueProperty() });

            FBindingConfiguration Configuration = Builder.Build();

            TArrayView<FResolvedViewModelEntry> ViewModels = Configuration.GetViewModels();
            TArrayView<FResolvedPropertyEntry> Properties = Configuration.GetProperties();

            TestEqual("Num ViewModels", ViewModels.Num(), 3);
            TestEqual("Num Properties", Properties.Num(), 3);

            TestViewModel("ViewModel[0]", ViewModels[0], { { UBindingWorkerViewModel_Root::StaticClass() }, 0, 1 });
            TestViewModel("ViewModel[1]", ViewModels[1], { { UBindingWorkerViewModel_FirstChild::StaticClass() }, 1, 1 });
            TestViewModel("ViewModel[2]", ViewModels[2], { { UBindingWorkerViewModel_SecondChild::StaticClass() }, 2, 1 });

            TestProperty("Property[0]", Properties[0], { UBindingWorkerViewModel_Root::ChildProperty(), 1 });
            TestProperty("Property[1]", Properties[1], { UBindingWorkerViewModel_FirstChild::ChildProperty(), 2 });
            TestProperty("Property[2]", Properties[2], { UBindingWorkerViewModel_SecondChild::IntValueProperty(), INDEX_NONE });
        });

        It("Should resolve bindings for multiple properties by path", [this]
        {
            FBindingConfigurationBuilder Builder(UBindingWorkerViewModel_Root::StaticClass());

            Builder.AddBinding({ UBindingWorkerViewModel_Root::ChildProperty(), UBindingWorkerViewModel_FirstChild::ChildProperty(), UBindingWorkerViewModel_SecondChild::IntValueProperty() });
            Builder.AddBinding({ UBindingWorkerViewModel_Root::ChildProperty(), UBindingWorkerViewModel_FirstChild::IntValueProperty() });

            FBindingConfiguration Configuration = Builder.Build();

            TArrayView<FResolvedViewModelEntry> ViewModels = Configuration.GetViewModels();
            TArrayView<FResolvedPropertyEntry> Properties = Configuration.GetProperties();

            TestEqual("Num ViewModels", ViewModels.Num(), 3);
            TestEqual("Num Properties", Properties.Num(), 4);

            TestViewModel("ViewModel[0]", ViewModels[0], { { UBindingWorkerViewModel_Root::StaticClass() }, 0, 1 });
            TestViewModel("ViewModel[1]", ViewModels[1], { { UBindingWorkerViewModel_FirstChild::StaticClass() }, 1, 2 });
            TestViewModel("ViewModel[2]", ViewModels[2], { { UBindingWorkerViewModel_SecondChild::StaticClass() }, 3, 1 });

            TestProperty("Property[0]", Properties[0], { UBindingWorkerViewModel_Root::ChildProperty(), 1 });
            TestProperty("Property[1]", Properties[1], { UBindingWorkerViewModel_FirstChild::ChildProperty(), 2 });
            TestProperty("Property[2]", Properties[2], { UBindingWorkerViewModel_FirstChild::IntValueProperty(), INDEX_NONE });
            TestProperty("Property[3]", Properties[3], { UBindingWorkerViewModel_SecondChild::IntValueProperty(), INDEX_NONE });
        });
    });

    Describe("Blueprint", [this]
    {
        It("Should resolve bindings for single property", [this]
        {
            FBindingConfigurationBuilder Builder(UBindingWorkerViewModel_Root::StaticClass());

            Builder.AddBinding({ FName("IntValue") });

            FBindingConfiguration Configuration = Builder.Build();

            TArrayView<FResolvedViewModelEntry> ViewModels = Configuration.GetViewModels();
            TArrayView<FResolvedPropertyEntry> Properties = Configuration.GetProperties();

            TestEqual("Num ViewModels", ViewModels.Num(), 1);
            TestEqual("Num Properties", Properties.Num(), 1);

            TestViewModel("ViewModel[0]", ViewModels[0], { { UBindingWorkerViewModel_Root::StaticClass() }, 0, 1 });
            TestProperty("Property[0]", Properties[0], { UBindingWorkerViewModel_Root::IntValueProperty(), INDEX_NONE });
        });

        It("Should resolve bindings for multiple properties", [this]
        {
            FBindingConfigurationBuilder Builder(UBindingWorkerViewModel_Root::StaticClass());

            Builder.AddBinding({ FName("IntValue") });
            Builder.AddBinding({ FName("MinIntValue") });
            Builder.AddBinding({ FName("MaxIntValue") });

            FBindingConfiguration Configuration = Builder.Build();

            TArrayView<FResolvedViewModelEntry> ViewModels = Configuration.GetViewModels();
            TArrayView<FResolvedPropertyEntry> Properties = Configuration.GetProperties();

            TestEqual("Num ViewModels", ViewModels.Num(), 1);
            TestEqual("Num Properties", Properties.Num(), 3);

            TestViewModel("ViewModel[0]", ViewModels[0], { { UBindingWorkerViewModel_Root::StaticClass() }, 0, 3 });

            TestProperty("Property[0]", Properties[0], { UBindingWorkerViewModel_Root::IntValueProperty(), INDEX_NONE });
            TestProperty("Property[1]", Properties[1], { UBindingWorkerViewModel_Root::MinIntValueProperty(), INDEX_NONE });
            TestProperty("Property[2]", Properties[2], { UBindingWorkerViewModel_Root::MaxIntValueProperty(), INDEX_NONE });
        });

        It("Should resolve bindings for single property by path", [this]
        {
            FBindingConfigurationBuilder Builder(UBindingWorkerViewModel_Root::StaticClass());

            Builder.AddBinding({ FName("Child"), FName("Child"), FName("IntValue") });

            FBindingConfiguration Configuration = Builder.Build();

            TArrayView<FResolvedViewModelEntry> ViewModels = Configuration.GetViewModels();
            TArrayView<FResolvedPropertyEntry> Properties = Configuration.GetProperties();

            TestEqual("Num ViewModels", ViewModels.Num(), 3);
            TestEqual("Num Properties", Properties.Num(), 3);

            TestViewModel("ViewModel[0]", ViewModels[0], { { UBindingWorkerViewModel_Root::StaticClass() }, 0, 1 });
            TestViewModel("ViewModel[1]", ViewModels[1], { { UBindingWorkerViewModel_FirstChild::StaticClass() }, 1, 1 });
            TestViewModel("ViewModel[2]", ViewModels[2], { { UBindingWorkerViewModel_SecondChild::StaticClass() }, 2, 1 });

            TestProperty("Property[0]", Properties[0], { UBindingWorkerViewModel_Root::ChildProperty(), 1 });
            TestProperty("Property[1]", Properties[1], { UBindingWorkerViewModel_FirstChild::ChildProperty(), 2 });
            TestProperty("Property[2]", Properties[2], { UBindingWorkerViewModel_SecondChild::IntValueProperty(), INDEX_NONE });
        });

        It("Should resolve bindings for multiple properties by path", [this]
        {
            FBindingConfigurationBuilder Builder(UBindingWorkerViewModel_Root::StaticClass());

            Builder.AddBinding({ FName("Child"), FName("Child"), FName("IntValue") });
            Builder.AddBinding({ FName("Child"), FName("IntValue") });

            FBindingConfiguration Configuration = Builder.Build();

            TArrayView<FResolvedViewModelEntry> ViewModels = Configuration.GetViewModels();
            TArrayView<FResolvedPropertyEntry> Properties = Configuration.GetProperties();

            TestEqual("Num ViewModels", ViewModels.Num(), 3);
            TestEqual("Num Properties", Properties.Num(), 4);

            TestViewModel("ViewModel[0]", ViewModels[0], { { UBindingWorkerViewModel_Root::StaticClass() }, 0, 1 });
            TestViewModel("ViewModel[1]", ViewModels[1], { { UBindingWorkerViewModel_FirstChild::StaticClass() }, 1, 2 });
            TestViewModel("ViewModel[2]", ViewModels[2], { { UBindingWorkerViewModel_SecondChild::StaticClass() }, 3, 1 });

            TestProperty("Property[0]", Properties[0], { UBindingWorkerViewModel_Root::ChildProperty(), 1 });
            TestProperty("Property[1]", Properties[1], { UBindingWorkerViewModel_FirstChild::ChildProperty(), 2 });
            TestProperty("Property[2]", Properties[2], { UBindingWorkerViewModel_FirstChild::IntValueProperty(), INDEX_NONE });
            TestProperty("Property[3]", Properties[3], { UBindingWorkerViewModel_SecondChild::IntValueProperty(), INDEX_NONE });
        });
    });
}

void FBindingConfigurationBuilderSpec::TestViewModel(const FString& Prefix, const UnrealMvvm_Impl::FResolvedViewModelEntry& Actual, const UnrealMvvm_Impl::FResolvedViewModelEntry& Expected)
{
    TestEqual(Prefix + " Class", Actual.ViewModelClass, Expected.ViewModelClass);
    TestEqual(Prefix + " FirstProperty", Actual.FirstProperty, Expected.FirstProperty);
    TestEqual(Prefix + " NumProperties", Actual.NumProperties, Expected.NumProperties);
}

void FBindingConfigurationBuilderSpec::TestProperty(const FString& Prefix, const UnrealMvvm_Impl::FResolvedPropertyEntry& Actual, const UnrealMvvm_Impl::FResolvedPropertyEntry& Expected)
{
    TestEqual(Prefix, Actual.Property, (const FViewModelPropertyBase*)Expected.Property);
    TestEqual(Prefix + " NextViewModel", Actual.NextViewModelIndex, Expected.NextViewModelIndex);
}
