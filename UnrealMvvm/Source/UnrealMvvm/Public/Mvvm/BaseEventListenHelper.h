// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

class FListenManager;

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

    FListenManager* Manager;
    TWidget* Widget;
    TEventPtr Event;
};