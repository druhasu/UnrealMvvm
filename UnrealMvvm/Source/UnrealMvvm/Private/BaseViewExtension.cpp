// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Mvvm/Impl/BaseView/BaseViewExtension.h"

void UBaseViewExtension::Construct()
{
    // View is constructed (i.e. visible), start listening and update current state
    InvokeStartListening(GetViewObject(), BindingWorker);
}

void UBaseViewExtension::Destruct()
{
    // View is no longer attached to anything, stop listening to ViewModel
    BindingWorker.StopListening();
}

UBaseViewExtension* UBaseViewExtension::Request(UUserWidget* Widget)
{
    if (Widget->HasAnyFlags(RF_ClassDefaultObject))
    {
        return nullptr;
    }

    UBaseViewExtension* Result = Widget->GetExtension<UBaseViewExtension>();

    if (!Result)
    {
        Result = Widget->AddExtension<UBaseViewExtension>();
        PrepareBindindsInternal(Widget, Result->BindingWorker);
    }

    return Result;
}

UBaseViewExtension* UBaseViewExtension::Get(const UUserWidget* Widget)
{
    return Widget->GetExtension<UBaseViewExtension>();
}
