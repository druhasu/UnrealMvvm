// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Mvvm/Impl/BaseView/BaseViewComponent.h"

void UBaseViewComponent::BeginPlay()
{
    Super::BeginPlay();

    // View is constructed, start listening and update current state
    InvokeStartListening(GetViewObject(), BindingWorker);
}

void UBaseViewComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    // View is no longer in play, stop listening to ViewModel
    BindingWorker.StopListening();
}

UBaseViewComponent* UBaseViewComponent::Request(AActor* Actor)
{
    if (Actor->HasAnyFlags(RF_ClassDefaultObject))
    {
        return nullptr;
    }

    UBaseViewComponent* Result = Actor->FindComponentByClass<UBaseViewComponent>();

    if (!Result)
    {
        Result = NewObject<UBaseViewComponent>(Actor);
        Result->RegisterComponent();
        PrepareBindindsInternal(Actor, Result->BindingWorker);
    }

    return Result;
}

UBaseViewComponent* UBaseViewComponent::Get(const AActor* Actor)
{
    return Actor->FindComponentByClass<UBaseViewComponent>();
}
