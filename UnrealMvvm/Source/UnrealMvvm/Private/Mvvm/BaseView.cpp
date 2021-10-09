// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Mvvm/BaseView.h"

void UBaseView::NativeConstruct()
{
    Super::NativeConstruct();
    ConstructedChanged.ExecuteIfBound(true);
    ListenEvents();
}

void UBaseView::NativeDestruct()
{
    UnsubscribeAll();
    ConstructedChanged.ExecuteIfBound(false);
    Super::NativeDestruct();
}