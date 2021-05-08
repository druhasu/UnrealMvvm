// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/PointerToMember.h"
#include "Templates/EnableIf.h"
#include "Delegates/Delegate.h"
#include <functional>

template<typename T1, typename T2> class TDynamicEventListenHelper;
template<typename T1, typename T2> class TSimpleEventListenHelper;

class UNREALMVVM_API FListenManager
{
public:
    /* Helper struct checking that given type is Dynamic Multicast Delegate */
    template<typename TEventPtr>
    struct TIsDynamicMulticastDelegate
    {
        using EventType = typename TDecay< typename TPointerToMember<TEventPtr>::ValueType >::Type;
        static const bool Value = TIsDerivedFrom< EventType, TMulticastScriptDelegate<> >::Value;
    };

    /* Helper struct checking that given type is NonDynamic Multicast Delegate */
    template<typename TEventPtr>
    struct TIsSimpleMulticastDelegate
    {
        using EventType = typename TDecay< typename TPointerToMember<TEventPtr>::ValueType >::Type;
        static const bool Value = TIsDerivedFrom< EventType, FMulticastDelegateBase<FWeakObjectPtr> >::Value;
    };

    /* Setup listening to Dynamic Multicast Delegate */
    template<typename TWidget, typename TEventPtr>
    typename TEnableIf< TIsDynamicMulticastDelegate<TEventPtr>::Value, TDynamicEventListenHelper<TWidget, TEventPtr> >::Type
    Listen(TWidget* Widget, TEventPtr Event);

    /* Setup listening to NonDynamic Multicast Delegate */
    template<typename TWidget, typename TEventPtr>
    typename TEnableIf< TIsSimpleMulticastDelegate<TEventPtr>::Value, TSimpleEventListenHelper<TWidget, TEventPtr> >::Type
    Listen(TWidget* Widget, TEventPtr Event);

    /* Removes all subscriptions */
    void UnsubscribeAll()
    {
        for (auto& Subscription : Subscriptions)
        {
            Subscription.Unsubscribe();
        }

        Subscriptions.Empty();
    }

private:
    template<typename T1, typename T2> friend class TDynamicEventListenHelper;
    template<typename T1, typename T2> friend class TSimpleEventListenHelper;

    template<typename TUnsubscriber, typename... TArgs>
    void AddSubscription(TArgs... Args)
    {
        int32 NewIndex = Subscriptions.AddUninitialized();
        FSubscription& Subscription = Subscriptions[NewIndex];
        new (Subscription.Buffer) TUnsubscriber(Args...);
    }

    struct FBaseUnsubscriber
    {
        virtual void Unsubscribe() = 0;
    };

    struct FSubscription
    {
        enum { StorageCapacity = sizeof(void*) * 3 };
        uint8 Buffer[StorageCapacity];

        void Unsubscribe()
        {
            reinterpret_cast<FBaseUnsubscriber*>(Buffer)->Unsubscribe();
        }
    };

    template<typename TWidget, typename TEventPtr>
    struct TObjectUnsubscriber : public FBaseUnsubscriber
    {
        TObjectUnsubscriber(TWidget* InWidget, TEventPtr InEvent, UObject* InSubscriber)
            : Widget(InWidget)
            , Event(InEvent)
            , Subscriber(InSubscriber)
        {
            static_assert(FSubscription::StorageCapacity >= sizeof(this), "Not enough storage for TObjectUnsubscriber");
        }

        void Unsubscribe() override
        {
            auto& EventRef = std::mem_fn(Event)(Widget);
            EventRef.RemoveAll(Subscriber);
        }

        TWidget* Widget;
        TEventPtr Event;
        UObject* Subscriber;
    };

    template<typename TWidget, typename TEventPtr>
    struct THandleUnsubscriber : public FBaseUnsubscriber
    {
        THandleUnsubscriber(TWidget* InWidget, TEventPtr InEvent, FDelegateHandle InHandle)
            : Widget(InWidget)
            , Event(InEvent)
            , Handle(InHandle)
        {
            static_assert(FSubscription::StorageCapacity >= sizeof(this), "Not enough storage for THandleUnsubscriber");
        }

        void Unsubscribe() override
        {
            auto& EventRef = std::mem_fn(Event)(Widget);
            EventRef.Remove(Handle);
        }

        TWidget* Widget;
        TEventPtr Event;
        FDelegateHandle Handle;
    };

    TArray<FSubscription> Subscriptions;
};

#include "Mvvm/DynamicEventListenHelper.h"
#include "Mvvm/SimpleEventListenHelper.h"

template<typename TWidget, typename TEventPtr>
typename TEnableIf< FListenManager::TIsDynamicMulticastDelegate<TEventPtr>::Value, TDynamicEventListenHelper<TWidget, TEventPtr> >::Type
FListenManager::Listen(TWidget* Widget, TEventPtr Event)
{
    return TDynamicEventListenHelper<TWidget, TEventPtr>(this, Widget, Event);
}

template<typename TWidget, typename TEventPtr>
typename TEnableIf< FListenManager::TIsSimpleMulticastDelegate<TEventPtr>::Value, TSimpleEventListenHelper<TWidget, TEventPtr> >::Type
FListenManager::Listen(TWidget* Widget, TEventPtr Event)
{
    return TSimpleEventListenHelper<TWidget, TEventPtr>(this, Widget, Event);
}