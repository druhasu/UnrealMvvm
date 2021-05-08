// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/BaseEventListenHelper.h"

template<typename TWidget, typename TEventPtr>
class TSimpleEventListenHelper : public TBaseEventListenHelper<TWidget, TEventPtr>
{
public:
    template<typename TCallback>
    void WithLambda(TCallback&& Callback)
    {
        // multicast delegate overload with lambda
        if (this->Widget != nullptr)
        {
            auto& EventRef = this->GetEvent();
            FDelegateHandle Handle = EventRef.AddLambda(MoveTemp(Callback));

            using SubscriptionType = FListenManager::THandleUnsubscriber<TWidget, TEventPtr>;
            this->Manager->template AddSubscription<SubscriptionType>(this->Widget, this->Event, Handle);
        }
    }

    template<typename TListener, typename TCallbackPtr>
    void WithUObject(TListener* Listener, TCallbackPtr Callback)
    {
        // multicast delegate overload with UObject
        if (this->Widget != nullptr)
        {
            auto& EventRef = this->GetEvent();
            FDelegateHandle Handle = EventRef.AddUObject(Listener, Callback);

            using SubscriptionType = FListenManager::THandleUnsubscriber<TWidget, TEventPtr>;
            this->Manager->template AddSubscription<SubscriptionType>(this->Widget, this->Event, Handle);
        }
    }

private:
    friend class FListenManager;

    TSimpleEventListenHelper(FListenManager* InManager, TWidget* InWidget, TEventPtr InEvent)
        : TBaseEventListenHelper<TWidget, TEventPtr>(InManager, InWidget, InEvent)
    {
    }
};