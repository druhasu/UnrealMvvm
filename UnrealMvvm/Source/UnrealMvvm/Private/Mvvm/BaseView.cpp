// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Mvvm/BaseView.h"

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