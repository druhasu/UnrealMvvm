// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/BaseEventListenHelper.h"
#include "Mvvm/PointerToMember.h"

template<typename TWidget, typename TEventPtr>
class TDynamicEventListenHelper : public TBaseEventListenHelper<TWidget, TEventPtr>
{
public:
    /* Do not use directly. Use 'WithDynamic' macro */
    template<typename TListener>
    void __WithDynamicImpl(TListener* Listener, typename TPointerToMember<TEventPtr>::ValueType::FDelegate::template TMethodPtrResolver<TListener>::FMethodPtr Callback, FName FunctionName)
    {
        if (Widget != nullptr)
        {
            auto& EventRef = (Widget->*Event);
            EventRef.__Internal_AddDynamic(Listener, Callback, FunctionName);

            using SubscriptionType = FListenManager::TObjectUnsubscriber<TWidget, TEventPtr>;
            Manager->AddSubscription<SubscriptionType>(Widget, Event, Listener);
        }
    }

private:
    friend class FListenManager;

    TDynamicEventListenHelper(FListenManager* InManager, TWidget* InWidget, TEventPtr InEvent)
        : TBaseEventListenHelper(InManager, InWidget, InEvent)
    {
    }
};

/* Adds Listener with UFunction */
#define WithDynamic(Listener, Function) __WithDynamicImpl(Listener, Function, STATIC_FUNCTION_FNAME( TEXT( #Function ) ) )