// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Misc/AutomationTest.h"

#include "Mvvm/Impl/Binding/BindingWorker.h"
#include "Mvvm/Impl/Binding/BindingConfigurationBuilder.h"
#include "BindingWorkerTestViewModel.h"
#include "BindingWorkerTestView.h"
#include "TempWorldHelper.h"

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
void TestPropertyPath(TFunctionRef<void(FBindingWorkerTestHandler& Handler, UBindingWorkerViewModel_Root* RootViewModel, UnrealMvvm_Impl::FBindingWorker& Worker)> TestFunction);
void TestPropertyPathNative(TFunctionRef<void(UBindingWorkerTestView* View, UBindingWorkerViewModel_Root* RootViewModel)> TestFunction);
void TestPropertyPathMultiple(const TArray<TArray<const FViewModelPropertyBase*>>& Bindings, TFunctionRef<void(UBindingWorkerViewModel_Root* ViewModel, const TArray<FBindingWorkerTestHandler*>& Handlers)> TestFunction);
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
            Worker.Init(nullptr, Configuration);
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
            Worker.Init(nullptr, Configuration);
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
            Worker.Init(nullptr, Configuration);
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
            Worker.Init(nullptr, Configuration);
            FBindingWorkerTestHandler& Handler1 = Worker.AddBindingHandler<FBindingWorkerTestHandler>({ UBindingWorkerViewModel_Root::IntValueProperty() });
            FBindingWorkerTestHandler& Handler2 = Worker.AddBindingHandler<FBindingWorkerTestHandler>({ UBindingWorkerViewModel_Root::IntValueProperty() });

            UBindingWorkerViewModel_Root* RootViewModel = NewObject<UBindingWorkerViewModel_Root>();
            Worker.SetViewModel(RootViewModel);

            Worker.StartListening();

            RootViewModel->SetIntValue(1);

            Handler1.TestCall(1, RootViewModel, UBindingWorkerViewModel_Root::IntValueProperty());
            Handler2.TestCall(1, RootViewModel, UBindingWorkerViewModel_Root::IntValueProperty());
        });

        It("Should handle value change after stopping and starting listening", [this]
        {
            FBindingConfigurationBuilder Builder(UBindingWorkerViewModel_Root::StaticClass());
            Builder.AddBinding({ UBindingWorkerViewModel_Root::IntValueProperty() });
            FBindingConfiguration Configuration = Builder.Build();

            FBindingWorker Worker;
            Worker.Init(nullptr, Configuration);
            FBindingWorkerTestHandler& Handler = Worker.AddBindingHandler<FBindingWorkerTestHandler>({ UBindingWorkerViewModel_Root::IntValueProperty() });

            UBindingWorkerViewModel_Root* RootViewModel = NewObject<UBindingWorkerViewModel_Root>();
            Worker.SetViewModel(RootViewModel);

            Worker.StartListening();
            Worker.StopListening();
            Worker.StartListening();

            RootViewModel->SetIntValue(1);

            Handler.TestCall(1, RootViewModel, UBindingWorkerViewModel_Root::IntValueProperty());
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
            Worker.Init(nullptr, Configuration);
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
            Worker.Init(nullptr, Configuration);
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

        It("Should handle value change after stopping and starting listening", [this]
        {
            FBindingConfigurationBuilder Builder(UBindingWorkerViewModel_Root::StaticClass());
            Builder.AddBinding({ UBindingWorkerViewModel_Root::IntValueProperty() });
            Builder.AddBinding({ UBindingWorkerViewModel_Root::MinIntValueProperty() });
            Builder.AddBinding({ UBindingWorkerViewModel_Root::MaxIntValueProperty() });
            FBindingConfiguration Configuration = Builder.Build();

            FBindingWorker Worker;
            Worker.Init(nullptr, Configuration);
            FBindingWorkerTestHandler& Handler = Worker.AddBindingHandler<FBindingWorkerTestHandler>({ UBindingWorkerViewModel_Root::IntValueProperty() });
            FBindingWorkerTestHandler& HandlerMin = Worker.AddBindingHandler<FBindingWorkerTestHandler>({ UBindingWorkerViewModel_Root::MinIntValueProperty() });
            FBindingWorkerTestHandler& HandlerMax = Worker.AddBindingHandler<FBindingWorkerTestHandler>({ UBindingWorkerViewModel_Root::MaxIntValueProperty() });

            UBindingWorkerViewModel_Root* RootViewModel = NewObject<UBindingWorkerViewModel_Root>();
            Worker.SetViewModel(RootViewModel);

            Worker.StartListening();
            Worker.StopListening();
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
            TestPropertyPath([this](FBindingWorkerTestHandler& Handler, UBindingWorkerViewModel_Root* RootViewModel, FBindingWorker& Worker)
            {
                UBaseViewModel* SecondChildViewModel = RootViewModel->GetChild()->GetChild();

                Handler.TestCall(0, SecondChildViewModel, UBindingWorkerViewModel_SecondChild::IntValueProperty());
            });
        });

        It("Should handle change of first Property in Path", [this]
        {
            TestPropertyPath([this](FBindingWorkerTestHandler& Handler, UBindingWorkerViewModel_Root* RootViewModel, FBindingWorker& Worker)
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
            TestPropertyPath([this](FBindingWorkerTestHandler& Handler, UBindingWorkerViewModel_Root* RootViewModel, FBindingWorker& Worker)
            {
                UBindingWorkerViewModel_FirstChild* FirstChildViewModel = RootViewModel->GetChild();

                UBindingWorkerViewModel_SecondChild* SecondChildAlternativeViewModel = NewObject<UBindingWorkerViewModel_SecondChild>();
                FirstChildViewModel->SetChild(SecondChildAlternativeViewModel);

                Handler.TestCall(1, SecondChildAlternativeViewModel, UBindingWorkerViewModel_SecondChild::IntValueProperty());
            });
        });

        It("Should handle change of last Property in Path", [this]
        {
            TestPropertyPath([this](FBindingWorkerTestHandler& Handler, UBindingWorkerViewModel_Root* RootViewModel, FBindingWorker& Worker)
            {
                UBindingWorkerViewModel_SecondChild* SecondChildViewModel = RootViewModel->GetChild()->GetChild();
                SecondChildViewModel->SetIntValue(1);

                Handler.TestCall(1, SecondChildViewModel, UBindingWorkerViewModel_SecondChild::IntValueProperty());
            });
        });

        It("Should handle change of last Property in Path after stopping and starting listening", [this]
        {
            TestPropertyPath([this](FBindingWorkerTestHandler& Handler, UBindingWorkerViewModel_Root* RootViewModel, FBindingWorker& Worker)
            {
                Worker.StopListening();
                Worker.StartListening();

                UBindingWorkerViewModel_SecondChild* SecondChildViewModel = RootViewModel->GetChild()->GetChild();
                SecondChildViewModel->SetIntValue(1);

                Handler.TestCall(1, SecondChildViewModel, UBindingWorkerViewModel_SecondChild::IntValueProperty());
            });
        });
    });

    Describe("PropertyPath Native", [this]
    {
        It("Should handle initial value", [this]
        {
            TestPropertyPathNative([this](UBindingWorkerTestView* View, UBindingWorkerViewModel_Root* RootViewModel)
            {
                TestEqual("RootValue", View->RootValue, RootViewModel->GetIntValue());
                TestEqual("FirstChildValue", View->FirstChildValue, RootViewModel->GetChild()->GetIntValue());
                TestEqual("SecondChildValue", View->SecondChildValue, RootViewModel->GetChild()->GetChild()->GetIntValue());
            });
        });

        It("Should handle change of all values", [this]
        {
            TestPropertyPathNative([this](UBindingWorkerTestView* View, UBindingWorkerViewModel_Root* RootViewModel)
            {
                UBindingWorkerViewModel_FirstChild* FirstChildViewModel = RootViewModel->GetChild();
                UBindingWorkerViewModel_SecondChild* SecondChildViewModel = RootViewModel->GetChild()->GetChild();

                RootViewModel->SetIntValue(11);
                FirstChildViewModel->SetIntValue(21);
                SecondChildViewModel->SetIntValue(31);

                TestEqual("RootValue", View->RootValue, RootViewModel->GetIntValue());
                TestEqual("FirstChildValue", View->FirstChildValue, FirstChildViewModel->GetIntValue());
                TestEqual("SecondChildValue", View->SecondChildValue, SecondChildViewModel->GetIntValue());
            });
        });

        It("Should handle change of first Property in Path", [this]
        {
            TestPropertyPathNative([this](UBindingWorkerTestView* View, UBindingWorkerViewModel_Root* RootViewModel)
            {
                UBindingWorkerViewModel_FirstChild* FirstChildAlternativeViewModel = NewObject<UBindingWorkerViewModel_FirstChild>();
                FirstChildAlternativeViewModel->SetIntValue(11);

                UBindingWorkerViewModel_SecondChild* SecondChildAlternativeViewModel = NewObject<UBindingWorkerViewModel_SecondChild>();
                SecondChildAlternativeViewModel->SetIntValue(21);

                FirstChildAlternativeViewModel->SetChild(SecondChildAlternativeViewModel);

                RootViewModel->SetChild(FirstChildAlternativeViewModel);

                TestEqual("RootValue", View->RootValue, RootViewModel->GetIntValue());
                TestEqual("FirstChildValue", View->FirstChildValue, RootViewModel->GetChild()->GetIntValue());
                TestEqual("SecondChildValue", View->SecondChildValue, RootViewModel->GetChild()->GetChild()->GetIntValue());
            });
        });

        It("Should handle change of middle Property in Path", [this]
        {
            TestPropertyPathNative([this](UBindingWorkerTestView* View, UBindingWorkerViewModel_Root* RootViewModel)
            {
                UBindingWorkerViewModel_FirstChild* FirstChildViewModel = RootViewModel->GetChild();

                UBindingWorkerViewModel_SecondChild* SecondChildAlternativeViewModel = NewObject<UBindingWorkerViewModel_SecondChild>();
                SecondChildAlternativeViewModel->SetIntValue(21);

                FirstChildViewModel->SetChild(SecondChildAlternativeViewModel);

                TestEqual("RootValue", View->RootValue, RootViewModel->GetIntValue());
                TestEqual("FirstChildValue", View->FirstChildValue, RootViewModel->GetChild()->GetIntValue());
                TestEqual("SecondChildValue", View->SecondChildValue, RootViewModel->GetChild()->GetChild()->GetIntValue());
            });
        });

        It("Should handle change of last Property in Path", [this]
        {
            TestPropertyPathNative([this](UBindingWorkerTestView* View, UBindingWorkerViewModel_Root* RootViewModel)
            {
                UBindingWorkerViewModel_SecondChild* SecondChildViewModel = RootViewModel->GetChild()->GetChild();
                SecondChildViewModel->SetIntValue(21);

                TestEqual("RootValue", View->RootValue, RootViewModel->GetIntValue());
                TestEqual("FirstChildValue", View->FirstChildValue, RootViewModel->GetChild()->GetIntValue());
                TestEqual("SecondChildValue", View->SecondChildValue, RootViewModel->GetChild()->GetChild()->GetIntValue());
            });
        });
    });

    Describe("PropertyPath multiple", [this]
    {
        It("Should handle change Root + Root", [this]
        {
            TestPropertyPathMultiple(
            {
                { UBindingWorkerViewModel_Root::ChildProperty() },
                { UBindingWorkerViewModel_Root::ChildProperty() },
            },
            [&](UBindingWorkerViewModel_Root* ViewModel, const TArray<FBindingWorkerTestHandler*>& Handlers)
            {
                ViewModel->SetChild(NewObject<UBindingWorkerViewModel_FirstChild>());

                TestEqual("Handler[0] Call count", Handlers[0]->Calls.Num(), 1);
                TestEqual("Handler[1] Call count", Handlers[1]->Calls.Num(), 1);

                Handlers[0]->TestCall(0, ViewModel, UBindingWorkerViewModel_Root::ChildProperty());
                Handlers[1]->TestCall(0, ViewModel, UBindingWorkerViewModel_Root::ChildProperty());
            });
        });

        It("Should handle change Child + Root", [this]
        {
            TestPropertyPathMultiple(
            {
                { UBindingWorkerViewModel_Root::ChildProperty(), UBindingWorkerViewModel_FirstChild::IntValueProperty() },
                { UBindingWorkerViewModel_Root::ChildProperty() },
            },
            [&](UBindingWorkerViewModel_Root* ViewModel, const TArray<FBindingWorkerTestHandler*>& Handlers)
            {
                ViewModel->SetChild(NewObject<UBindingWorkerViewModel_FirstChild>());

                TestEqual("Handler[0] Call count", Handlers[0]->Calls.Num(), 1);
                TestEqual("Handler[1] Call count", Handlers[1]->Calls.Num(), 1);

                Handlers[0]->TestCall(0, ViewModel->GetChild(), UBindingWorkerViewModel_FirstChild::IntValueProperty());
                Handlers[1]->TestCall(0, ViewModel, UBindingWorkerViewModel_Root::ChildProperty());
            });
        });

        It("Should handle change Root + Child", [this]
        {
            TestPropertyPathMultiple(
            {
                { UBindingWorkerViewModel_Root::ChildProperty() },
                { UBindingWorkerViewModel_Root::ChildProperty(), UBindingWorkerViewModel_FirstChild::IntValueProperty() },
            },
            [&](UBindingWorkerViewModel_Root* ViewModel, const TArray<FBindingWorkerTestHandler*>& Handlers)
            {
                ViewModel->SetChild(NewObject<UBindingWorkerViewModel_FirstChild>());

                TestEqual("Handler[0] Call count", Handlers[0]->Calls.Num(), 1);
                TestEqual("Handler[1] Call count", Handlers[1]->Calls.Num(), 1);

                Handlers[0]->TestCall(0, ViewModel, UBindingWorkerViewModel_Root::ChildProperty());
                Handlers[1]->TestCall(0, ViewModel->GetChild(), UBindingWorkerViewModel_FirstChild::IntValueProperty());
            });
        });

        It("Should handle change Child + Child", [this]
        {
            TestPropertyPathMultiple(
            {
                { UBindingWorkerViewModel_Root::ChildProperty(), UBindingWorkerViewModel_FirstChild::IntValueProperty() },
                { UBindingWorkerViewModel_Root::ChildProperty(), UBindingWorkerViewModel_FirstChild::IntValueProperty() },
            },
            [&](UBindingWorkerViewModel_Root* ViewModel, const TArray<FBindingWorkerTestHandler*>& Handlers)
            {
                ViewModel->SetChild(NewObject<UBindingWorkerViewModel_FirstChild>());

                TestEqual("Handler[0] Call count", Handlers[0]->Calls.Num(), 1);
                TestEqual("Handler[1] Call count", Handlers[1]->Calls.Num(), 1);

                Handlers[0]->TestCall(0, ViewModel->GetChild(), UBindingWorkerViewModel_FirstChild::IntValueProperty());
                Handlers[1]->TestCall(0, ViewModel->GetChild(), UBindingWorkerViewModel_FirstChild::IntValueProperty());
            });
        });
    });
}

void FBindingWorkerSpec::TestPropertyPath(TFunctionRef<void(FBindingWorkerTestHandler& Handler, UBindingWorkerViewModel_Root* RootViewModel, UnrealMvvm_Impl::FBindingWorker& Worker)> TestFunction)
{
    using namespace UnrealMvvm_Impl;

    FBindingConfigurationBuilder Builder(UBindingWorkerViewModel_Root::StaticClass());
    Builder.AddBinding({ UBindingWorkerViewModel_Root::ChildProperty(), UBindingWorkerViewModel_FirstChild::ChildProperty(), UBindingWorkerViewModel_SecondChild::IntValueProperty() });
    FBindingConfiguration Configuration = Builder.Build();

    FBindingWorker Worker;
    Worker.Init(nullptr, Configuration);

    UBindingWorkerViewModel_Root* RootViewModel = NewObject<UBindingWorkerViewModel_Root>();
    UBindingWorkerViewModel_FirstChild* FirstChildViewModel = NewObject<UBindingWorkerViewModel_FirstChild>();
    UBindingWorkerViewModel_SecondChild* SecondChildViewModel = NewObject<UBindingWorkerViewModel_SecondChild>();

    RootViewModel->SetChild(FirstChildViewModel);
    FirstChildViewModel->SetChild(SecondChildViewModel);

    Worker.SetViewModel(RootViewModel);

    FBindingWorkerTestHandler& Handler = Worker.AddBindingHandler<FBindingWorkerTestHandler>({ UBindingWorkerViewModel_Root::ChildProperty(), UBindingWorkerViewModel_FirstChild::ChildProperty(), UBindingWorkerViewModel_SecondChild::IntValueProperty() });
    Worker.StartListening();

    TestFunction(Handler, RootViewModel, Worker);
}

void FBindingWorkerSpec::TestPropertyPathNative(TFunctionRef<void(UBindingWorkerTestView* View, UBindingWorkerViewModel_Root* RootViewModel)> TestFunction)
{
    using namespace UnrealMvvm_Impl;

    FTempWorldHelper Helper;

    UBindingWorkerTestView* View = CreateWidget<UBindingWorkerTestView>(Helper.World);

    UBindingWorkerViewModel_Root* RootViewModel = NewObject<UBindingWorkerViewModel_Root>();
    UBindingWorkerViewModel_FirstChild* FirstChildViewModel = NewObject<UBindingWorkerViewModel_FirstChild>();
    UBindingWorkerViewModel_SecondChild* SecondChildViewModel = NewObject<UBindingWorkerViewModel_SecondChild>();

    RootViewModel->SetChild(FirstChildViewModel);
    FirstChildViewModel->SetChild(SecondChildViewModel);

    // initialize values to not match defaults from UBindingWorkerTestView
    RootViewModel->SetIntValue(123);
    FirstChildViewModel->SetIntValue(123);
    SecondChildViewModel->SetIntValue(123);

    View->SetViewModel(RootViewModel);
    TSharedPtr<SWidget> SWidgetPtr = View->TakeWidget();

    TestFunction(View, RootViewModel);
}

void FBindingWorkerSpec::TestPropertyPathMultiple(const TArray<TArray<const FViewModelPropertyBase*>>& Bindings, TFunctionRef<void(UBindingWorkerViewModel_Root* ViewModel, const TArray<FBindingWorkerTestHandler*>& Handlers)> TestFunction)
{
    using namespace UnrealMvvm_Impl;

    FBindingConfigurationBuilder Builder(UBindingWorkerViewModel_Root::StaticClass());
    for (const TArray<const FViewModelPropertyBase*>& Binding : Bindings)
    {
        Builder.AddBinding(Binding);
    }

    FBindingWorker Worker;
    Worker.Init(nullptr, Builder.Build());

    UBindingWorkerViewModel_Root* RootViewModel = NewObject<UBindingWorkerViewModel_Root>();
    Worker.SetViewModel(RootViewModel);

    TArray<FBindingWorkerTestHandler*> Handlers;
    for (const TArray<const FViewModelPropertyBase*>& Binding : Bindings)
    {
        Handlers.Add(&Worker.AddBindingHandler<FBindingWorkerTestHandler>(Binding));
    }

    Worker.StartListening();

    // reset calls produced by StartListening to not mess with what TestFunction expects
    for (FBindingWorkerTestHandler* Handler : Handlers)
    {
        Handler->Calls.Reset();
    }

    TestFunction(RootViewModel, Handlers);
}
