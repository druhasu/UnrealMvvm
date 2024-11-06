// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/Impl/ListenManager/BaseEventListenHelper.h"
#include "Mvvm/Impl/Utils/PointerToMember.h"

namespace UnrealMvvm_Impl
{

    template<typename TWidget, typename TEventPtr>
    class TDynamicEventListenHelper : public TBaseEventListenHelper<TWidget, TEventPtr>
    {
    public:
        /* Do not use directly. Use 'WithDynamic' macro */
        template<typename TListener>
        void __WithDynamicImpl(TListener* Listener, typename TDecay<typename TPointerToMember<TEventPtr>::ValueType>::Type::FDelegate::template TMethodPtrResolver<TListener>::FMethodPtr Callback, FName FunctionName)
        {
            if (this->Widget != nullptr)
            {
                auto& EventRef = this->GetEvent();
                EventRef.__Internal_AddDynamic(Listener, Callback, FunctionName);

                using SubscriptionType = typename FListenManager::TObjectUnsubscriber<TWidget, TEventPtr>;
                this->Manager->template AddSubscription<SubscriptionType>(this->Widget, this->Event, Listener);
            }
        }

    private:
        friend class ::FListenManager;

        TDynamicEventListenHelper(FListenManager* InManager, TWidget* InWidget, TEventPtr InEvent)
            : TBaseEventListenHelper<TWidget, TEventPtr>(InManager, InWidget, InEvent)
        {
        }
    };

    /* Adds Listener with UFunction */
    #define WithDynamic(Listener, Function) __WithDynamicImpl(Listener, Function, STATIC_FUNCTION_FNAME( TEXT( #Function ) ) )

}
