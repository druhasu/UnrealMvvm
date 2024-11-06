// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/Impl/ListenManager/BaseEventListenHelper.h"

namespace UnrealMvvm_Impl
{

    template<typename TWidget, typename TEventPtr>
    class TSimpleEventListenHelper : public TBaseEventListenHelper<TWidget, TEventPtr>
    {
    public:
        template<typename TCallback>
        void WithStatic(TCallback&& Callback)
        {
            if (this->Widget != nullptr)
            {
                AddSubscription(this->GetEvent().AddStatic(MoveTemp(Callback)));
            }
        }

        template<typename TCallback>
        void WithLambda(TCallback&& Callback)
        {
            if (this->Widget != nullptr)
            {
                AddSubscription(this->GetEvent().AddLambda(MoveTemp(Callback)));
            }
        }

        template<typename TObj, typename TCallback>
        void WithWeakLambda(TObj* Obj, TCallback&& Callback)
        {
            if (this->Widget != nullptr)
            {
                AddSubscription(this->GetEvent().AddWeakLambda(Obj, MoveTemp(Callback)));
            }
        }

        template<typename TListener, typename TCallbackPtr>
        void WithSP(TListener* Listener, TCallbackPtr Callback)
        {
            if (this->Widget != nullptr)
            {
                AddSubscription(this->GetEvent().AddSP(Listener, Callback));
            }
        }

        template<typename TListener, typename TCallbackPtr>
        void WithUObject(TListener* Listener, TCallbackPtr Callback)
        {
            if (this->Widget != nullptr)
            {
                AddSubscription(this->GetEvent().AddUObject(Listener, Callback));
            }
        }

    private:
        friend class ::FListenManager;

        TSimpleEventListenHelper(FListenManager* InManager, TWidget* InWidget, TEventPtr InEvent)
            : TBaseEventListenHelper<TWidget, TEventPtr>(InManager, InWidget, InEvent)
        {
        }

        void AddSubscription(const FDelegateHandle& Handle)
        {
            using SubscriptionType = FListenManager::THandleUnsubscriber<TWidget, TEventPtr>;
            this->Manager->template AddSubscription<SubscriptionType>(this->Widget, this->Event, Handle);
        }
    };

}