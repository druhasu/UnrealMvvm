// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Mvvm/BaseView.h"
#include "Mvvm/Impl/BaseViewExtension.h"
#include "Mvvm/Impl/BaseViewClassExtension.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"

bool UBaseView::Initialize()
{
    if (!HasAnyFlags(RF_ClassDefaultObject))
    {
        UWidgetBlueprintGeneratedClass* WidgetClass = Cast<UWidgetBlueprintGeneratedClass>(GetClass());
        UBaseViewClassExtension* ClassExtension = WidgetClass ? WidgetClass->GetExtension<UBaseViewClassExtension>() : nullptr;

        if (!ClassExtension)
        {
            // if there is no UBaseViewClassExtension, we need to manually add UBaseViewExtension to itself
            AddExtension(UBaseViewExtension::StaticClass());
        }
    }

    return Super::Initialize();
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