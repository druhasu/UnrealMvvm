// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Mvvm/Impl/ViewModelDynamicBinding.h"
#include "Mvvm/Impl/ViewModelRegistry.h"

void UViewModelDynamicBinding::Serialize(FArchive& Ar)
{
    Super::Serialize(Ar);

    if (Ar.IsLoading())
    {
        if (ViewModelClass)
        {
            // ViewModel class may be removed, thus we get nullptr here
            // in this case do not register class until it is fixed
            UnrealMvvm_Impl::FViewModelRegistry::RegisterViewClass(Cast<UClass>(GetOuter()), ViewModelClass);
        }
    }
}