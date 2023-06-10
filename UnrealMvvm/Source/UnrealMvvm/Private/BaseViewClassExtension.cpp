// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Mvvm/Impl/BaseViewClassExtension.h"
#include "Mvvm/Impl/BaseViewExtension.h"
#include "Mvvm/Impl/ViewModelRegistry.h"

void UBaseViewClassExtension::Initialize(UUserWidget* UserWidget)
{
    UBaseViewExtension::Request(UserWidget);
}

void UBaseViewClassExtension::PreConstruct(UUserWidget* UserWidget, bool IsDesignTime)
{
    // unfortunately, Initialize is not called if created widget is not a child of another UUserWidget
    // and, because we have to create Extension before NativeConstruct, we have only one place left - PreConstruct
    if (!IsDesignTime)
    {
        UBaseViewExtension::Request(UserWidget);
    }
}

void UBaseViewClassExtension::PostLoad()
{
    if (ViewModelClass)
    {
        // ViewModel class may be removed, thus we get nullptr here
        // in this case do not register class until it is fixed
        UnrealMvvm_Impl::FViewModelRegistry::RegisterViewClass(Cast<UClass>(GetOuter()), ViewModelClass);
    }

    Super::PostLoad();
}