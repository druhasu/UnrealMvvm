// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Mvvm/BaseView.h"
#include "Mvvm/Impl/BaseView/BaseViewExtension.h"

void UBaseView::NativePreConstruct()
{
    Super::NativePreConstruct();

    if (!IsDesignTime())
    {
        UBaseViewExtension::Request(this);
    }
}

void UBaseView::NativeConstruct()
{
    Super::NativeConstruct();
    ListenEvents();
}

void UBaseView::NativeDestruct()
{
    UnsubscribeAll();
    Super::NativeDestruct();
}
