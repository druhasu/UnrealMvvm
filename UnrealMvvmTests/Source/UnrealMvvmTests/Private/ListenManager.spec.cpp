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
    FTestEvent EventField;
    FTestEvent& EventMethod() { return EventField; }

    DECLARE_MULTICAST_DELEGATE(FTestDelegate);
    FTestDelegate DelegateField;
    FTestDelegate& DelegateMethod() { return DelegateField; }

    FTestDynamicDelegate DynamicDelegateField;
    FTestDynamicDelegate& DynamicDelegateMethod() { return DynamicDelegateField; }
};

void ListenManagerSpec::Define()
{
    Describe("Simple Delegate", [this]()
    {
        Describe("WithLambda", [this]()
        {
            It("Should Listen To Simple Event Field With Lambda", [this]()
            {
                FListenManager Manager;
                FEventHolder Holder;
                bool Invoked = false;

                Manager.Listen(&Holder, &FEventHolder::EventField).WithLambda([&Invoked]() { Invoked = true; });
                Holder.EventField.Broadcast();

                TestTrue("Listener not added", Holder.EventField.IsBound());
                TestTrue("Listener not invoked", Invoked);
            });

            It("Should Listen To Simple Delegate Field With Lambda", [this]()
            {
                FListenManager Manager;
                FEventHolder Holder;
                bool Invoked = false;

                Manager.Listen(&Holder, &FEventHolder::DelegateField).WithLambda([&Invoked]() { Invoked = true; });
                Holder.DelegateField.Broadcast();

                TestTrue("Listener not added", Holder.DelegateField.IsBound());
                TestTrue("Listener not invoked", Invoked);
            });

            It("Should Listen To Simple Event Method With Lambda", [this]()
            {
                FListenManager Manager;
                FEventHolder Holder;
                bool Invoked = false;

                Manager.Listen(&Holder, &FEventHolder::EventMethod).WithLambda([&Invoked]() { Invoked = true; });
                Holder.EventField.Broadcast();

                TestTrue("Listener not added", Holder.EventField.IsBound());
                TestTrue("Listener not invoked", Invoked);
            });

            It("Should Listen To Simple Delegate Method With Lambda", [this]()
            {
                FListenManager Manager;
                FEventHolder Holder;
                bool Invoked = false;

                Manager.Listen(&Holder, &FEventHolder::DelegateMethod).WithLambda([&Invoked]() { Invoked = true; });
                Holder.DelegateField.Broadcast();

                TestTrue("Listener not added", Holder.DelegateField.IsBound());
                TestTrue("Listener not invoked", Invoked);
            });

            It("Should Unsubscribe From Simple Event With Lambda", [this]()
            {
                FListenManager Manager;
                FEventHolder Holder;

                Manager.Listen(&Holder, &FEventHolder::EventField).WithLambda([]() {});
                TestTrue("Listener not added", Holder.EventField.IsBound());

                Manager.UnsubscribeAll();
                TestFalse("Listener not removed", Holder.EventField.IsBound());
            });

            It("Should Unsubscribe From Simple Delegate With Lambda", [this]()
            {
                FListenManager Manager;
                FEventHolder Holder;

                Manager.Listen(&Holder, &FEventHolder::DelegateField).WithLambda([]() {});
                TestTrue("Listener not added", Holder.DelegateField.IsBound());

                Manager.UnsubscribeAll();
                TestFalse("Listener not removed", Holder.DelegateField.IsBound());
            });
        });

        Describe("WithUObject", [this]()
        {
            It("Should Listen To Simple Event Field With UObject", [this]()
            {
                FListenManager Manager;
                FEventHolder Holder;
                UTestListener* Listener = NewObject<UTestListener>();

                Manager.Listen(&Holder, &FEventHolder::EventField).WithUObject(Listener, &UTestListener::SimpleCallback);
                Holder.EventField.Broadcast();

                TestTrue("Listener not added", Holder.EventField.IsBound());
                TestTrue("Listener not invoked", Listener->Invoked);
            });

            It("Should Listen To Simple Delegate Field With UObject", [this]()
            {
                FListenManager Manager;
                FEventHolder Holder;
                UTestListener* Listener = NewObject<UTestListener>();

                Manager.Listen(&Holder, &FEventHolder::DelegateField).WithUObject(Listener, &UTestListener::SimpleCallback);
                Holder.DelegateField.Broadcast();

                TestTrue("Listener not added", Holder.DelegateField.IsBound());
                TestTrue("Listener not invoked", Listener->Invoked);
            });

            It("Should Listen To Simple Event Method With UObject", [this]()
            {
                FListenManager Manager;
                FEventHolder Holder;
                UTestListener* Listener = NewObject<UTestListener>();

                Manager.Listen(&Holder, &FEventHolder::EventMethod).WithUObject(Listener, &UTestListener::SimpleCallback);
                Holder.EventField.Broadcast();

                TestTrue("Listener not added", Holder.EventField.IsBound());
                TestTrue("Listener not invoked", Listener->Invoked);
            });

            It("Should Listen To Simple Delegate Method With UObject", [this]()
            {
                FListenManager Manager;
                FEventHolder Holder;
                UTestListener* Listener = NewObject<UTestListener>();

                Manager.Listen(&Holder, &FEventHolder::DelegateMethod).WithUObject(Listener, &UTestListener::SimpleCallback);
                Holder.DelegateField.Broadcast();

                TestTrue("Listener not added", Holder.DelegateField.IsBound());
                TestTrue("Listener not invoked", Listener->Invoked);
            });

            It("Should Unsubscribe From Simple Event With UObject", [this]()
            {
                FListenManager Manager;
                FEventHolder Holder;
                UTestListener* Listener = NewObject<UTestListener>();

                Manager.Listen(&Holder, &FEventHolder::EventField).WithUObject(Listener, &UTestListener::SimpleCallback);
                TestTrue("Listener not added", Holder.EventField.IsBound());

                Manager.UnsubscribeAll();
                TestFalse("Listener not removed", Holder.EventField.IsBound());
            });

            It("Should Unsubscribe From Simple Delegate With UObject", [this]()
            {
                FListenManager Manager;
                FEventHolder Holder;
                UTestListener* Listener = NewObject<UTestListener>();

                Manager.Listen(&Holder, &FEventHolder::DelegateField).WithUObject(Listener, &UTestListener::SimpleCallback);
                TestTrue("Listener not added", Holder.DelegateField.IsBound());

                Manager.UnsubscribeAll();
                TestFalse("Listener not removed", Holder.DelegateField.IsBound());
            });
        });
    });

    Describe("Dynamic Delegate", [this]()
    {
        Describe("WithDynamic", [this]()
        {
            It("Should Listen To Dynamic Delegate Field With UFunction", [this]()
            {
                FListenManager Manager;
                FEventHolder Holder;
                UTestListener* Listener = NewObject<UTestListener>();

                Manager.Listen(&Holder, &FEventHolder::DynamicDelegateField).WithDynamic(Listener, &UTestListener::DynamicCallback);
                Holder.DynamicDelegateField.Broadcast();

                TestTrue("Listener not added", Holder.DynamicDelegateField.IsBound());
                TestTrue("Listener not invoked", Listener->Invoked);
            });

            It("Should Listen To Dynamic Delegate Method With UFunction", [this]()
            {
                FListenManager Manager;
                FEventHolder Holder;
                UTestListener* Listener = NewObject<UTestListener>();

                Manager.Listen(&Holder, &FEventHolder::DynamicDelegateMethod).WithDynamic(Listener, &UTestListener::DynamicCallback);
                Holder.DynamicDelegateField.Broadcast();

                TestTrue("Listener not added", Holder.DynamicDelegateField.IsBound());
                TestTrue("Listener not invoked", Listener->Invoked);
            });

            It("Should Unsubscribe From Dynamic Delegate With UFunction", [this]()
            {
                FListenManager Manager;
                FEventHolder Holder;
                UTestListener* Listener = NewObject<UTestListener>();

                Manager.Listen(&Holder, &FEventHolder::DynamicDelegateField).WithDynamic(Listener, &UTestListener::DynamicCallback);
                TestTrue("Listener not added", Holder.DynamicDelegateField.IsBound());

                Manager.UnsubscribeAll();
                TestFalse("Listener not removed", Holder.DynamicDelegateField.IsBound());
            });
        });
    });
}