// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/Impl/ListenManager/BaseEventListenHelper.h"
#include "Mvvm/Impl/Utils/PointerToMember.h"
#include "Misc/EngineVersionComparison.h"

namespace UnrealMvvm_Impl
{

    template<typename TWidget, typename TEventPtr>
    class TDynamicEventListenHelper : public TBaseEventListenHelper<TWidget, TEventPtr>
    {
    public:
        /* Do not use directly. Use 'WithDynamic' macro */
        template<typename TListener>
#if UE_VERSION_OLDER_THAN(5,8,0)
        void __WithDynamicImpl(TListener* Listener, typename TDecay<typename TPointerToMember<TEventPtr>::ValueType>::Type::FDelegate::template TMethodPtrResolver<TListener>::FMethodPtr Callback, FName FunctionName)
#else
        void __WithDynamicImpl(TListener* Listener, typename TDecay<typename TPointerToMember<TEventPtr>::ValueType>::Type::FDelegate::template TMethodPtrResolver<false, TListener>::FMethodPtr Callback, FName FunctionName)
#endif
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
#if UE_VERSION_OLDER_THAN(5,7,0)
    #define WithDynamic(Listener, Function) __WithDynamicImpl(Listener, Function, STATIC_FUNCTION_FNAME( TEXT( #Function ) ) )
#elif UE_VERSION_OLDER_THAN(5,8,0)
    #define WithDynamic(Listener, Function) __WithDynamicImpl(Listener, Function, STATIC_FUNCTION_FNAME( #Function ) )
#else
    #define WithDynamic(Listener, Function) __WithDynamicImpl(Listener, Function, UE_PRIVATE_STATIC_FUNCTION_FNAME( #Function ) )
#endif

}
