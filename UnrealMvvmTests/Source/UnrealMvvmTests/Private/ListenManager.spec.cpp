// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Misc/AutomationTest.h"

#include "Mvvm/ListenManager.h"

#include "TestListener.h"

BEGIN_DEFINE_SPEC(ListenManagerSpec, "UnrealMvvm.ListenManager", EAutomationTestFlags::ClientContext | EAutomationTestFlags::EditorContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
END_DEFINE_SPEC(ListenManagerSpec)

class FEventHolder
{
public:
    DECLARE_EVENT(FEventHolder, FTestEvent);
    mutable FTestEvent EventField;
    FTestEvent& EventMethod() { return EventField; }
    FTestEvent& EventMethodConst() const { return EventField; }

    DECLARE_MULTICAST_DELEGATE(FTestDelegate);
    mutable FTestDelegate DelegateField;
    FTestDelegate& DelegateMethod() { return DelegateField; }
    FTestDelegate& DelegateMethodConst() const { return DelegateField; }

    mutable FTestDynamicDelegate DynamicDelegateField;
    FTestDynamicDelegate& DynamicDelegateMethod() { return DynamicDelegateField; }
    FTestDynamicDelegate& DynamicDelegateMethodConst() const { return DynamicDelegateField; }
};

class FSecondBase
{
    int32 Dummy;
};

class FMultiInheritedEventHolder : public FEventHolder, public FSecondBase
{
public:
    FTestEvent EventFieldDerived;
    FTestEvent& EventMethodDerived() { return EventFieldDerived; }

    FTestDynamicDelegate DynamicDelegateFieldDerived;
    FTestDynamicDelegate& DynamicDelegateMethodDerived() { return DynamicDelegateFieldDerived; }
};

bool StaticInvoked = false;
void StaticCallback()
{
    StaticInvoked = true;
}

void ListenManagerSpec::Define()
{
    Describe("Simple Delegate", [this]()
    {
        Describe("WithStatic", [this]()
        {
            It("Should Listen To Simple Event Field With Static", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;
                StaticInvoked = false;

                Manager.Listen(&Holder, &FEventHolder::EventField).WithStatic(&StaticCallback);
                Holder.EventField.Broadcast();

                TestTrue("Listener added", Holder.EventField.IsBound());
                TestTrue("Listener invoked", StaticInvoked);
            });

            It("Should Listen To Simple Delegate Field With Static", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;
                StaticInvoked = false;

                Manager.Listen(&Holder, &FEventHolder::DelegateField).WithStatic(&StaticCallback);
                Holder.DelegateField.Broadcast();

                TestTrue("Listener added", Holder.DelegateField.IsBound());
                TestTrue("Listener invoked", StaticInvoked);
            });

            It("Should Listen To Simple Event Method With Static", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;
                StaticInvoked = false;

                Manager.Listen(&Holder, &FEventHolder::EventMethod).WithStatic(&StaticCallback);
                Holder.EventField.Broadcast();

                TestTrue("Listener added", Holder.EventField.IsBound());
                TestTrue("Listener invoked", StaticInvoked);
            });

            It("Should Listen To Simple Delegate Method With Static", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;
                StaticInvoked = false;

                Manager.Listen(&Holder, &FEventHolder::DelegateMethod).WithStatic(&StaticCallback);
                Holder.DelegateField.Broadcast();

                TestTrue("Listener added", Holder.DelegateField.IsBound());
                TestTrue("Listener invoked", StaticInvoked);
            });

            It("Should Listen To Simple Event Const Method With Static", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;
                StaticInvoked = false;

                Manager.Listen(&Holder, &FEventHolder::EventMethodConst).WithStatic(&StaticCallback);
                Holder.EventField.Broadcast();

                TestTrue("Listener added", Holder.EventField.IsBound());
                TestTrue("Listener invoked", StaticInvoked);
            });

            It("Should Listen To Simple Delegate Const Method With Static", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;
                StaticInvoked = false;

                Manager.Listen(&Holder, &FEventHolder::DelegateMethodConst).WithStatic(&StaticCallback);
                Holder.DelegateField.Broadcast();

                TestTrue("Listener added", Holder.DelegateField.IsBound());
                TestTrue("Listener invoked", StaticInvoked);
            });

            It("Should Unsubscribe From Simple Event With Static", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;

                Manager.Listen(&Holder, &FEventHolder::EventField).WithStatic(&StaticCallback);
                TestTrue("Listener added", Holder.EventField.IsBound());

                Manager.UnsubscribeAll();
                TestFalse("Listener removed", Holder.EventField.IsBound());
            });

            It("Should Unsubscribe From Simple Delegate With Static", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;

                Manager.Listen(&Holder, &FEventHolder::DelegateField).WithStatic(&StaticCallback);
                TestTrue("Listener added", Holder.DelegateField.IsBound());

                Manager.UnsubscribeAll();
                TestFalse("Listener removed", Holder.DelegateField.IsBound());
            });
        });

        Describe("WithLambda", [this]()
        {
            It("Should Listen To Simple Event Field With Lambda", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;
                bool Invoked = false;

                Manager.Listen(&Holder, &FEventHolder::EventField).WithLambda([&Invoked]() { Invoked = true; });
                Holder.EventField.Broadcast();

                TestTrue("Listener added", Holder.EventField.IsBound());
                TestTrue("Listener invoked", Invoked);
            });

            It("Should Listen To Simple Delegate Field With Lambda", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;
                bool Invoked = false;

                Manager.Listen(&Holder, &FEventHolder::DelegateField).WithLambda([&Invoked]() { Invoked = true; });
                Holder.DelegateField.Broadcast();

                TestTrue("Listener added", Holder.DelegateField.IsBound());
                TestTrue("Listener invoked", Invoked);
            });

            It("Should Listen To Simple Event Method With Lambda", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;
                bool Invoked = false;

                Manager.Listen(&Holder, &FEventHolder::EventMethod).WithLambda([&Invoked]() { Invoked = true; });
                Holder.EventField.Broadcast();

                TestTrue("Listener added", Holder.EventField.IsBound());
                TestTrue("Listener invoked", Invoked);
            });

            It("Should Listen To Simple Delegate Method With Lambda", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;
                bool Invoked = false;

                Manager.Listen(&Holder, &FEventHolder::DelegateMethod).WithLambda([&Invoked]() { Invoked = true; });
                Holder.DelegateField.Broadcast();

                TestTrue("Listener added", Holder.DelegateField.IsBound());
                TestTrue("Listener invoked", Invoked);
            });

            It("Should Listen To Simple Event Const Method With Lambda", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;
                bool Invoked = false;

                Manager.Listen(&Holder, &FEventHolder::EventMethodConst).WithLambda([&Invoked]() { Invoked = true; });
                Holder.EventField.Broadcast();

                TestTrue("Listener added", Holder.EventField.IsBound());
                TestTrue("Listener invoked", Invoked);
            });

            It("Should Listen To Simple Delegate Const Method With Lambda", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;
                bool Invoked = false;

                Manager.Listen(&Holder, &FEventHolder::DelegateMethodConst).WithLambda([&Invoked]() { Invoked = true; });
                Holder.DelegateField.Broadcast();

                TestTrue("Listener added", Holder.DelegateField.IsBound());
                TestTrue("Listener invoked", Invoked);
            });

            It("Should Unsubscribe From Simple Event With Lambda", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;

                Manager.Listen(&Holder, &FEventHolder::EventField).WithLambda([]() {});
                TestTrue("Listener added", Holder.EventField.IsBound());

                Manager.UnsubscribeAll();
                TestFalse("Listener removed", Holder.EventField.IsBound());
            });

            It("Should Unsubscribe From Simple Delegate With Lambda", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;

                Manager.Listen(&Holder, &FEventHolder::DelegateField).WithLambda([]() {});
                TestTrue("Listener added", Holder.DelegateField.IsBound());

                Manager.UnsubscribeAll();
                TestFalse("Listener removed", Holder.DelegateField.IsBound());
            });
        });

        Describe("WithWeakLambda", [this]()
        {
            It("Should Listen To Simple Event Field With Lambda", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;
                bool Invoked = false;
                UTestListener* Listener = NewObject<UTestListener>();

                Manager.Listen(&Holder, &FEventHolder::EventField).WithWeakLambda(Listener, [&Invoked]() { Invoked = true; });
                Holder.EventField.Broadcast();

                TestTrue("Listener added", Holder.EventField.IsBound());
                TestTrue("Listener invoked", Invoked);
            });

            It("Should Listen To Simple Delegate Field With WeakLambda", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;
                bool Invoked = false;
                UTestListener* Listener = NewObject<UTestListener>();

                Manager.Listen(&Holder, &FEventHolder::DelegateField).WithWeakLambda(Listener, [&Invoked]() { Invoked = true; });
                Holder.DelegateField.Broadcast();

                TestTrue("Listener added", Holder.DelegateField.IsBound());
                TestTrue("Listener invoked", Invoked);
            });

            It("Should Listen To Simple Event Method With WeakLambda", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;
                bool Invoked = false;
                UTestListener* Listener = NewObject<UTestListener>();

                Manager.Listen(&Holder, &FEventHolder::EventMethod).WithWeakLambda(Listener, [&Invoked]() { Invoked = true; });
                Holder.EventField.Broadcast();

                TestTrue("Listener added", Holder.EventField.IsBound());
                TestTrue("Listener invoked", Invoked);
            });

            It("Should Listen To Simple Delegate Method With WeakLambda", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;
                bool Invoked = false;
                UTestListener* Listener = NewObject<UTestListener>();

                Manager.Listen(&Holder, &FEventHolder::DelegateMethod).WithWeakLambda(Listener, [&Invoked]() { Invoked = true; });
                Holder.DelegateField.Broadcast();

                TestTrue("Listener added", Holder.DelegateField.IsBound());
                TestTrue("Listener invoked", Invoked);
            });

            It("Should Listen To Simple Event Const Method With WeakLambda", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;
                bool Invoked = false;
                UTestListener* Listener = NewObject<UTestListener>();

                Manager.Listen(&Holder, &FEventHolder::EventMethodConst).WithWeakLambda(Listener, [&Invoked]() { Invoked = true; });
                Holder.EventField.Broadcast();

                TestTrue("Listener added", Holder.EventField.IsBound());
                TestTrue("Listener invoked", Invoked);
            });

            It("Should Listen To Simple Delegate Const Method With WeakLambda", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;
                bool Invoked = false;
                UTestListener* Listener = NewObject<UTestListener>();

                Manager.Listen(&Holder, &FEventHolder::DelegateMethodConst).WithWeakLambda(Listener, [&Invoked]() { Invoked = true; });
                Holder.DelegateField.Broadcast();

                TestTrue("Listener added", Holder.DelegateField.IsBound());
                TestTrue("Listener invoked", Invoked);
            });

            It("Should Unsubscribe From Simple Event With WeakLambda", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;
                UTestListener* Listener = NewObject<UTestListener>();

                Manager.Listen(&Holder, &FEventHolder::EventField).WithWeakLambda(Listener, []() {});
                TestTrue("Listener added", Holder.EventField.IsBound());

                Manager.UnsubscribeAll();
                TestFalse("Listener removed", Holder.EventField.IsBound());
            });

            It("Should Unsubscribe From Simple Delegate With WeakLambda", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;
                UTestListener* Listener = NewObject<UTestListener>();

                Manager.Listen(&Holder, &FEventHolder::DelegateField).WithWeakLambda(Listener, []() {});
                TestTrue("Listener added", Holder.DelegateField.IsBound());

                Manager.UnsubscribeAll();
                TestFalse("Listener removed", Holder.DelegateField.IsBound());
            });
        });

        Describe("WithSP", [this]()
        {
            It("Should Listen To Simple Event Field With SharedPtr", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;
                TSharedPtr<FTestListener> Listener = MakeShared<FTestListener>();

                Manager.Listen(&Holder, &FEventHolder::EventField).WithSP(Listener.Get(), &FTestListener::SimpleCallback);
                Holder.EventField.Broadcast();

                TestTrue("Listener added", Holder.EventField.IsBound());
                TestTrue("Listener invoked", Listener->Invoked);
            });

            It("Should Listen To Simple Delegate Field With SharedPtr", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;
                TSharedPtr<FTestListener> Listener = MakeShared<FTestListener>();

                Manager.Listen(&Holder, &FEventHolder::DelegateField).WithSP(Listener.Get(), &FTestListener::SimpleCallback);
                Holder.DelegateField.Broadcast();

                TestTrue("Listener added", Holder.DelegateField.IsBound());
                TestTrue("Listener invoked", Listener->Invoked);
            });

            It("Should Listen To Simple Event Method With SharedPtr", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;
                TSharedPtr<FTestListener> Listener = MakeShared<FTestListener>();

                Manager.Listen(&Holder, &FEventHolder::EventMethod).WithSP(Listener.Get(), &FTestListener::SimpleCallback);
                Holder.EventField.Broadcast();

                TestTrue("Listener added", Holder.EventField.IsBound());
                TestTrue("Listener invoked", Listener->Invoked);
            });

            It("Should Listen To Simple Delegate Method With SharedPtr", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;
                TSharedPtr<FTestListener> Listener = MakeShared<FTestListener>();

                Manager.Listen(&Holder, &FEventHolder::DelegateMethod).WithSP(Listener.Get(), &FTestListener::SimpleCallback);
                Holder.DelegateField.Broadcast();

                TestTrue("Listener added", Holder.DelegateField.IsBound());
                TestTrue("Listener invoked", Listener->Invoked);
            });

            It("Should Listen To Simple Event Const Method With SharedPtr", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;
                TSharedPtr<FTestListener> Listener = MakeShared<FTestListener>();

                Manager.Listen(&Holder, &FEventHolder::EventMethodConst).WithSP(Listener.Get(), &FTestListener::SimpleCallback);
                Holder.EventField.Broadcast();

                TestTrue("Listener added", Holder.EventField.IsBound());
                TestTrue("Listener invoked", Listener->Invoked);
            });

            It("Should Listen To Simple Delegate Const Method With SharedPtr", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;
                TSharedPtr<FTestListener> Listener = MakeShared<FTestListener>();

                Manager.Listen(&Holder, &FEventHolder::DelegateMethodConst).WithSP(Listener.Get(), &FTestListener::SimpleCallback);
                Holder.DelegateField.Broadcast();

                TestTrue("Listener added", Holder.DelegateField.IsBound());
                TestTrue("Listener invoked", Listener->Invoked);
            });

            It("Should Unsubscribe From Simple Event With SharedPtr", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;
                TSharedPtr<FTestListener> Listener = MakeShared<FTestListener>();

                Manager.Listen(&Holder, &FEventHolder::EventField).WithSP(Listener.Get(), &FTestListener::SimpleCallback);
                TestTrue("Listener added", Holder.EventField.IsBound());

                Manager.UnsubscribeAll();
                TestFalse("Listener removed", Holder.EventField.IsBound());
            });

            It("Should Unsubscribe From Simple Delegate With SharedPtr", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;
                TSharedPtr<FTestListener> Listener = MakeShared<FTestListener>();

                Manager.Listen(&Holder, &FEventHolder::DelegateField).WithSP(Listener.Get(), &FTestListener::SimpleCallback);
                TestTrue("Listener added", Holder.DelegateField.IsBound());

                Manager.UnsubscribeAll();
                TestFalse("Listener removed", Holder.DelegateField.IsBound());
            });
        });

        Describe("WithUObject", [this]()
        {
            It("Should Listen To Simple Event Field With UObject", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;
                UTestListener* Listener = NewObject<UTestListener>();

                Manager.Listen(&Holder, &FEventHolder::EventField).WithUObject(Listener, &UTestListener::SimpleCallback);
                Holder.EventField.Broadcast();

                TestTrue("Listener added", Holder.EventField.IsBound());
                TestTrue("Listener invoked", Listener->Invoked);
            });

            It("Should Listen To Simple Delegate Field With UObject", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;
                UTestListener* Listener = NewObject<UTestListener>();

                Manager.Listen(&Holder, &FEventHolder::DelegateField).WithUObject(Listener, &UTestListener::SimpleCallback);
                Holder.DelegateField.Broadcast();

                TestTrue("Listener added", Holder.DelegateField.IsBound());
                TestTrue("Listener invoked", Listener->Invoked);
            });

            It("Should Listen To Simple Event Method With UObject", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;
                UTestListener* Listener = NewObject<UTestListener>();

                Manager.Listen(&Holder, &FEventHolder::EventMethod).WithUObject(Listener, &UTestListener::SimpleCallback);
                Holder.EventField.Broadcast();

                TestTrue("Listener added", Holder.EventField.IsBound());
                TestTrue("Listener invoked", Listener->Invoked);
            });

            It("Should Listen To Simple Delegate Method With UObject", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;
                UTestListener* Listener = NewObject<UTestListener>();

                Manager.Listen(&Holder, &FEventHolder::DelegateMethod).WithUObject(Listener, &UTestListener::SimpleCallback);
                Holder.DelegateField.Broadcast();

                TestTrue("Listener added", Holder.DelegateField.IsBound());
                TestTrue("Listener invoked", Listener->Invoked);
            });

            It("Should Listen To Simple Event Const Method With UObject", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;
                UTestListener* Listener = NewObject<UTestListener>();

                Manager.Listen(&Holder, &FEventHolder::EventMethodConst).WithUObject(Listener, &UTestListener::SimpleCallback);
                Holder.EventField.Broadcast();

                TestTrue("Listener added", Holder.EventField.IsBound());
                TestTrue("Listener invoked", Listener->Invoked);
            });

            It("Should Listen To Simple Delegate Const Method With UObject", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;
                UTestListener* Listener = NewObject<UTestListener>();

                Manager.Listen(&Holder, &FEventHolder::DelegateMethodConst).WithUObject(Listener, &UTestListener::SimpleCallback);
                Holder.DelegateField.Broadcast();

                TestTrue("Listener added", Holder.DelegateField.IsBound());
                TestTrue("Listener invoked", Listener->Invoked);
            });

            It("Should Unsubscribe From Simple Event With UObject", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;
                UTestListener* Listener = NewObject<UTestListener>();

                Manager.Listen(&Holder, &FEventHolder::EventField).WithUObject(Listener, &UTestListener::SimpleCallback);
                TestTrue("Listener added", Holder.EventField.IsBound());

                Manager.UnsubscribeAll();
                TestFalse("Listener removed", Holder.EventField.IsBound());
            });

            It("Should Unsubscribe From Simple Delegate With UObject", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;
                UTestListener* Listener = NewObject<UTestListener>();

                Manager.Listen(&Holder, &FEventHolder::DelegateField).WithUObject(Listener, &UTestListener::SimpleCallback);
                TestTrue("Listener added", Holder.DelegateField.IsBound());

                Manager.UnsubscribeAll();
                TestFalse("Listener removed", Holder.DelegateField.IsBound());
            });
        });

        It("Should Compile Unsubscriber From Event Of Class With Several Bases", [this]()
        {
            FMultiInheritedEventHolder Holder;
            FListenManager Manager;
            bool Invoked = false;

            Manager.Listen(&Holder, &FMultiInheritedEventHolder::EventFieldDerived).WithLambda([&Invoked]() { Invoked = true; });
            Manager.Listen(&Holder, &FMultiInheritedEventHolder::EventMethodDerived).WithLambda([&Invoked]() { Invoked = true; });
            Holder.EventField.Broadcast();
            Manager.UnsubscribeAll();
        });

        It("Should Support TObjectPtr", [this]
        {
            TObjectPtr<UTestEventHolder> Holder = NewObject<UTestEventHolder>();
            FListenManager Manager;
            bool Invoked = false;

            Manager.Listen(Holder, &UTestEventHolder::DelegateField).WithLambda([&Invoked]() { Invoked = true; });
            Holder->DelegateField.Broadcast();

            TestTrue("Listener added", Holder->DelegateField.IsBound());
            TestTrue("Listener invoked", Invoked);
        });
    });

    Describe("Dynamic Delegate", [this]()
    {
        Describe("WithDynamic", [this]()
        {
            It("Should Listen To Dynamic Delegate Field With UFunction", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;
                UTestListener* Listener = NewObject<UTestListener>();

                Manager.Listen(&Holder, &FEventHolder::DynamicDelegateField).WithDynamic(Listener, &UTestListener::DynamicCallback);
                Holder.DynamicDelegateField.Broadcast();

                TestTrue("Listener added", Holder.DynamicDelegateField.IsBound());
                TestTrue("Listener invoked", Listener->Invoked);
            });

            It("Should Listen To Dynamic Delegate Method With UFunction", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;
                UTestListener* Listener = NewObject<UTestListener>();

                Manager.Listen(&Holder, &FEventHolder::DynamicDelegateMethod).WithDynamic(Listener, &UTestListener::DynamicCallback);
                Holder.DynamicDelegateField.Broadcast();

                TestTrue("Listener added", Holder.DynamicDelegateField.IsBound());
                TestTrue("Listener invoked", Listener->Invoked);
            });

            It("Should Listen To Dynamic Delegate Const Method With UFunction", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;
                UTestListener* Listener = NewObject<UTestListener>();

                Manager.Listen(&Holder, &FEventHolder::DynamicDelegateMethodConst).WithDynamic(Listener, &UTestListener::DynamicCallback);
                Holder.DynamicDelegateField.Broadcast();

                TestTrue("Listener added", Holder.DynamicDelegateField.IsBound());
                TestTrue("Listener invoked", Listener->Invoked);
            });

            It("Should Unsubscribe From Dynamic Delegate With UFunction", [this]()
            {
                FEventHolder Holder;
                FListenManager Manager;
                UTestListener* Listener = NewObject<UTestListener>();

                Manager.Listen(&Holder, &FEventHolder::DynamicDelegateField).WithDynamic(Listener, &UTestListener::DynamicCallback);
                TestTrue("Listener added", Holder.DynamicDelegateField.IsBound());

                Manager.UnsubscribeAll();
                TestFalse("Listener removed", Holder.DynamicDelegateField.IsBound());
            });
        });

        It("Should Compile Unsubscriber From Event Of Class With Several Bases", [this]()
        {
            FListenManager Manager;
            UTestListener* Listener = NewObject<UTestListener>();

            {
                FMultiInheritedEventHolder Holder;
                Manager.Listen(&Holder, &FMultiInheritedEventHolder::DynamicDelegateFieldDerived).WithDynamic(Listener, &UTestListener::DynamicCallback);
                Holder.DynamicDelegateField.Broadcast();
                Manager.UnsubscribeAll();
            }
            {
                FMultiInheritedEventHolder Holder;
                Manager.Listen(&Holder, &FMultiInheritedEventHolder::DynamicDelegateMethodDerived).WithDynamic(Listener, &UTestListener::DynamicCallback);
                Holder.DynamicDelegateField.Broadcast();
                Manager.UnsubscribeAll();
            }
        });

        It("Should Support TObjectPtr", [this]
        {
            TObjectPtr<UTestEventHolder> Holder = NewObject<UTestEventHolder>();
            FListenManager Manager;
            UTestListener* Listener = NewObject<UTestListener>();

            Manager.Listen(Holder, &UTestEventHolder::DynamicDelegateField).WithDynamic(Listener, &UTestListener::DynamicCallback);
            Holder->DynamicDelegateField.Broadcast();

            TestTrue("Listener added", Holder->DynamicDelegateField.IsBound());
            TestTrue("Listener invoked", Listener->Invoked);
        });
    });

    It("Should Remove All Subscriptions From Destructor", [this]()
    {
        FEventHolder Holder;
        {
            FListenManager Manager;
            Manager.Listen(&Holder, &FEventHolder::DelegateField).WithLambda([]() {});
            TestTrue("Listener added", Holder.DelegateField.IsBound());
        }

        TestFalse("Listener not removed", Holder.DelegateField.IsBound());
    });
}