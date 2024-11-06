// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Mvvm/Impl/Binding/ViewModelDynamicBinding.h"
#include "Mvvm/Impl/BaseView/ViewRegistry.h"

void UViewModelDynamicBinding::PostLoad()
{
    Super::PostLoad();

    if (ViewModelClass)
    {
        // ViewModel class may be removed, thus we get nullptr here
        // in this case do not register class until it is fixed
        UnrealMvvm_Impl::FViewRegistry::RegisterViewClass(Cast<UClass>(GetOuter()), ViewModelClass);
    }
}
