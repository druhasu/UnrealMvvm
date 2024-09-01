// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include <functional>

class FListenManager;

namespace UnrealMvvm_Impl
{

    template<typename TWidget, typename TEventPtr>
    class TBaseEventListenHelper
    {
    protected:
        TBaseEventListenHelper(FListenManager* InManager, TWidget* InWidget, TEventPtr InEvent)
            : Manager(InManager)
            , Widget(InWidget)
            , Event(InEvent)
        {
        }

        auto& GetEvent() const
        {
            return std::mem_fn(Event)(Widget);
        }

        FListenManager* Manager;
        TWidget* Widget;
        TEventPtr Event;
    };

}