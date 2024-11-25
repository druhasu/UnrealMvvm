// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Misc/AutomationTest.h"

#include "Mvvm/Impl/Binding/BindingWorker.h"
#include "Mvvm/Impl/Binding/BindingConfigurationBuilder.h"
#include "BindingWorkerTestViewModel.h"

struct FBindingWorkerTestHandler : public UnrealMvvm_Impl::IPropertyChangeHandler
{
    void Invoke(UBaseViewModel* ViewModel, const FViewModelPropertyBase* Property) const override
    {
        Calls.Add(MakeTuple(ViewModel, Property));
    }

    void TestCall(int32 Index, UBaseViewModel* ViewModel, const FViewModelPropertyBase* Property)
    {
        FAutomationTestBase* Test = FAutomationTestFramework::Get().GetCurrentTest();

        if (Test->TestTrue(FString::Printf(TEXT("Has Call[%d]"), Index), Calls.IsValidIndex(Index)))
        {
            Test->TestEqual(FString::Printf(TEXT("Calls[%d].ViewModel"), Index), Calls[Index].Key, ViewModel);
            Test->TestEqual(FString::Printf(TEXT("Calls[%d].Property"), Index), Calls[Index].Value, Property);
        }
    }

    mutable TArray<TTuple<UBaseViewModel*, const FViewModelPropertyBase*>> Calls;
};

BEGIN_DEFINE_SPEC(FBindingWorkerSpec, "UnrealMvvm.BindingWorker", EAutomationTestFlags::ClientContext | EAutomationTestFlags::EditorContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
void TestPropertyPath(TFunctionRef<void(FBindingWorkerTestHandler& Handler, UBindingWorkerViewModel_Root* RootViewModel)> TestFunction);
END_DEFINE_SPEC(FBindingWorkerSpec)

void FBindingWorkerSpec::Define()
{
    using namespace UnrealMvvm_Impl;

    Describe("Single Property", [this]
    {
        It("Should handle initial value", [this]
        {
            FBindingConfigurationBuilder Builder(UBindingWorkerViewModel_Root::StaticClass());
            Builder.AddBinding({ UBindingWorkerViewModel_Root::IntValueProperty() });
            FBindingConfiguration Configuration = Builder.Build();

            FBindingWorker Worker;
            Worker.Init(Configuration);
            FBindingWorkerTestHandler& Handler = Worker.AddBindingHandler<FBindingWorkerTestHandler>({ UBindingWorkerViewModel_Root::IntValueProperty() });

            UBindingWorkerViewModel_Root* RootViewModel = NewObject<UBindingWorkerViewModel_Root>();
            Worker.SetViewModel(RootViewModel);

            Worker.StartListening();

            Handler.TestCall(0, RootViewModel, UBindingWorkerViewModel_Root::IntValueProperty());
        });

        It("Should handle initial value with multiple handlers", [this]
        {
            FBindingConfigurationBuilder Builder(UBindingWorkerViewModel_Root::StaticClass());
            Builder.AddBinding({ UBindingWorkerViewModel_Root::IntValueProperty() });
            Builder.AddBinding({ UBindingWorkerViewModel_Root::IntValueProperty() });
            FBindingConfiguration Configuration = Builder.Build();

            FBindingWorker Worker;
            Worker.Init(Configuration);
            FBindingWorkerTestHandler& Handler1 = Worker.AddBindingHandler<FBindingWorkerTestHandler>({ UBindingWorkerViewModel_Root::IntValueProperty() });
            FBindingWorkerTestHandler& Handler2 = Worker.AddBindingHandler<FBindingWorkerTestHandler>({ UBindingWorkerViewModel_Root::IntValueProperty() });

            UBindingWorkerViewModel_Root* RootViewModel = NewObject<UBindingWorkerViewModel_Root>();
            Worker.SetViewModel(RootViewModel);

            Worker.StartListening();

            Handler1.TestCall(0, RootViewModel, UBindingWorkerViewModel_Root::IntValueProperty());
            Handler2.TestCall(0, RootViewModel, UBindingWorkerViewModel_Root::IntValueProperty());
        });

        It("Should handle value change", [this]
        {
            FBindingConfigurationBuilder Builder(UBindingWorkerViewModel_Root::StaticClass());
            Builder.AddBinding({ UBindingWorkerViewModel_Root::IntValueProperty() });
            FBindingConfiguration Configuration = Builder.Build();

            FBindingWorker Worker;
            Worker.Init(Configuration);
            FBindingWorkerTestHandler& Handler = Worker.AddBindingHandler<FBindingWorkerTestHandler>({ UBindingWorkerViewModel_Root::IntValueProperty() });

            UBindingWorkerViewModel_Root* RootViewModel = NewObject<UBindingWorkerViewModel_Root>();
            Worker.SetViewModel(RootViewModel);

            Worker.StartListening();

            RootViewModel->SetIntValue(1);

            Handler.TestCall(1, RootViewModel, UBindingWorkerViewModel_Root::IntValueProperty());
        });

        It("Should handle value change with multiple handlers", [this]
        {
            FBindingConfigurationBuilder Builder(UBindingWorkerViewModel_Root::StaticClass());
            Builder.AddBinding({ UBindingWorkerViewModel_Root::IntValueProperty() });
            Builder.AddBinding({ UBindingWorkerViewModel_Root::IntValueProperty() });
            FBindingConfiguration Configuration = Builder.Build();

            FBindingWorker Worker;
            Worker.Init(Configuration);
            FBindingWorkerTestHandler& Handler1 = Worker.AddBindingHandler<FBindingWorkerTestHandler>({ UBindingWorkerViewModel_Root::IntValueProperty() });
            FBindingWorkerTestHandler& Handler2 = Worker.AddBindingHandler<FBindingWorkerTestHandler>({ UBindingWorkerViewModel_Root::IntValueProperty() });

            UBindingWorkerViewModel_Root* RootViewModel = NewObject<UBindingWorkerViewModel_Root>();
            Worker.SetViewModel(RootViewModel);

            Worker.StartListening();

            RootViewModel->SetIntValue(1);

            Handler1.TestCall(1, RootViewModel, UBindingWorkerViewModel_Root::IntValueProperty());
            Handler2.TestCall(1, RootViewModel, UBindingWorkerViewModel_Root::IntValueProperty());
        });
    });

    Describe("Multiple properties", [this]
    {
        It("Should handle initial value", [this]
        {
            FBindingConfigurationBuilder Builder(UBindingWorkerViewModel_Root::StaticClass());
            Builder.AddBinding({ UBindingWorkerViewModel_Root::IntValueProperty() });
            Builder.AddBinding({ UBindingWorkerViewModel_Root::MinIntValueProperty() });
            Builder.AddBinding({ UBindingWorkerViewModel_Root::MaxIntValueProperty() });
            FBindingConfiguration Configuration = Builder.Build();

            FBindingWorker Worker;
            Worker.Init(Configuration);
            FBindingWorkerTestHandler& Handler = Worker.AddBindingHandler<FBindingWorkerTestHandler>({ UBindingWorkerViewModel_Root::IntValueProperty() });
            FBindingWorkerTestHandler& HandlerMin = Worker.AddBindingHandler<FBindingWorkerTestHandler>({ UBindingWorkerViewModel_Root::MinIntValueProperty() });
            FBindingWorkerTestHandler& HandlerMax = Worker.AddBindingHandler<FBindingWorkerTestHandler>({ UBindingWorkerViewModel_Root::MaxIntValueProperty() });

            UBindingWorkerViewModel_Root* RootViewModel = NewObject<UBindingWorkerViewModel_Root>();
            Worker.SetViewModel(RootViewModel);

            Worker.StartListening();

            Handler.TestCall(0, RootViewModel, UBindingWorkerViewModel_Root::IntValueProperty());
            HandlerMin.TestCall(0, RootViewModel, UBindingWorkerViewModel_Root::MinIntValueProperty());
            HandlerMax.TestCall(0, RootViewModel, UBindingWorkerViewModel_Root::MaxIntValueProperty());
        });

        It("Should handle value change", [this]
        {
            FBindingConfigurationBuilder Builder(UBindingWorkerViewModel_Root::StaticClass());
            Builder.AddBinding({ UBindingWorkerViewModel_Root::IntValueProperty() });
            Builder.AddBinding({ UBindingWorkerViewModel_Root::MinIntValueProperty() });
            Builder.AddBinding({ UBindingWorkerViewModel_Root::MaxIntValueProperty() });
            FBindingConfiguration Configuration = Builder.Build();

            FBindingWorker Worker;
            Worker.Init(Configuration);
            FBindingWorkerTestHandler& Handler = Worker.AddBindingHandler<FBindingWorkerTestHandler>({ UBindingWorkerViewModel_Root::IntValueProperty() });
            FBindingWorkerTestHandler& HandlerMin = Worker.AddBindingHandler<FBindingWorkerTestHandler>({ UBindingWorkerViewModel_Root::MinIntValueProperty() });
            FBindingWorkerTestHandler& HandlerMax = Worker.AddBindingHandler<FBindingWorkerTestHandler>({ UBindingWorkerViewModel_Root::MaxIntValueProperty() });

            UBindingWorkerViewModel_Root* RootViewModel = NewObject<UBindingWorkerViewModel_Root>();
            Worker.SetViewModel(RootViewModel);

            Worker.StartListening();

            RootViewModel->SetIntValue(1);
            RootViewModel->SetMinIntValue(1);
            RootViewModel->SetMaxIntValue(1);

            Handler.TestCall(1, RootViewModel, UBindingWorkerViewModel_Root::IntValueProperty());
            HandlerMin.TestCall(1, RootViewModel, UBindingWorkerViewModel_Root::MinIntValueProperty());
            HandlerMax.TestCall(1, RootViewModel, UBindingWorkerViewModel_Root::MaxIntValueProperty());
        });
    });

    Describe("PropertyPath", [this]
    {
        It("Should handle initial value", [this]
        {
            TestPropertyPath([this](FBindingWorkerTestHandler& Handler, UBindingWorkerViewModel_Root* RootViewModel)
            {
                UBaseViewModel* SecondChildViewModel = RootViewModel->GetChild()->GetChild();

                Handler.TestCall(0, SecondChildViewModel, UBindingWorkerViewModel_SecondChild::IntValueProperty());
            });
        });

        It("Should handle change of first Property in Path", [this]
        {
            TestPropertyPath([this](FBindingWorkerTestHandler& Handler, UBindingWorkerViewModel_Root* RootViewModel)
            {
                UBindingWorkerViewModel_FirstChild* FirstChildAlternativeViewModel = NewObject<UBindingWorkerViewModel_FirstChild>();
                UBindingWorkerViewModel_SecondChild* SecondChildAlternativeViewModel = NewObject<UBindingWorkerViewModel_SecondChild>();
                FirstChildAlternativeViewModel->SetChild(SecondChildAlternativeViewModel);

                RootViewModel->SetChild(FirstChildAlternativeViewModel);

                Handler.TestCall(1, SecondChildAlternativeViewModel, UBindingWorkerViewModel_SecondChild::IntValueProperty());
            });
        });

        It("Should handle change of middle Property in Path", [this]
        {
            TestPropertyPath([this](FBindingWorkerTestHandler& Handler, UBindingWorkerViewModel_Root* RootViewModel)
            {
                UBindingWorkerViewModel_FirstChild* FirstChildViewModel = RootViewModel->GetChild();

                UBindingWorkerViewModel_SecondChild* SecondChildAlternativeViewModel = NewObject<UBindingWorkerViewModel_SecondChild>();
                FirstChildViewModel->SetChild(SecondChildAlternativeViewModel);

                Handler.TestCall(1, SecondChildAlternativeViewModel, UBindingWorkerViewModel_SecondChild::IntValueProperty());
            });
        });

        It("Should handle change of last Property in Path", [this]
        {
            TestPropertyPath([this](FBindingWorkerTestHandler& Handler, UBindingWorkerViewModel_Root* RootViewModel)
            {
                UBindingWorkerViewModel_SecondChild* SecondChildViewModel = RootViewModel->GetChild()->GetChild();
                SecondChildViewModel->SetIntValue(1);

                Handler.TestCall(1, SecondChildViewModel, UBindingWorkerViewModel_SecondChild::IntValueProperty());
            });
        });
    });
}

void FBindingWorkerSpec::TestPropertyPath(TFunctionRef<void(FBindingWorkerTestHandler& Handler, UBindingWorkerViewModel_Root* RootViewModel)> TestFunction)
{
    using namespace UnrealMvvm_Impl;

    FBindingConfigurationBuilder Builder(UBindingWorkerViewModel_Root::StaticClass());
    Builder.AddBinding({ UBindingWorkerViewModel_Root::ChildProperty(), UBindingWorkerViewModel_FirstChild::ChildProperty(), UBindingWorkerViewModel_SecondChild::IntValueProperty() });
    FBindingConfiguration Configuration = Builder.Build();

    UnrealMvvm_Impl::FBindingWorker Worker;
    Worker.Init(Configuration);

    UBindingWorkerViewModel_Root* RootViewModel = NewObject<UBindingWorkerViewModel_Root>();
    UBindingWorkerViewModel_FirstChild* FirstChildViewModel = NewObject<UBindingWorkerViewModel_FirstChild>();
    UBindingWorkerViewModel_SecondChild* SecondChildViewModel = NewObject<UBindingWorkerViewModel_SecondChild>();

    RootViewModel->SetChild(FirstChildViewModel);
    FirstChildViewModel->SetChild(SecondChildViewModel);

    Worker.SetViewModel(RootViewModel);

    FBindingWorkerTestHandler& Handler = Worker.AddBindingHandler<FBindingWorkerTestHandler>({ UBindingWorkerViewModel_Root::ChildProperty(), UBindingWorkerViewModel_FirstChild::ChildProperty(), UBindingWorkerViewModel_SecondChild::IntValueProperty() });
    Worker.StartListening();

    TestFunction(Handler, RootViewModel);
}
