// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Mvvm/Impl/MvvmBlueprintLibrary.h"
#include "Mvvm/Impl/ViewModelRegistry.h"
#include "Mvvm/BaseView.h"

void UMvvmBlueprintLibrary::GetViewModelPropertyValue(UBaseView* View, FName PropertyName, int32& Value)
{
    checkNoEntry();
}

DEFINE_FUNCTION(UMvvmBlueprintLibrary::execGetViewModelPropertyValue)
{
    P_GET_OBJECT(UBaseView, View);
    P_GET_PROPERTY(FNameProperty, PropertyName);

    Stack.StepCompiledIn<FProperty>(nullptr);
    void* OutValuePtr = Stack.MostRecentPropertyAddress;

    if (View)
    {
        UBaseViewModel* ViewModel = View->ViewModel;
        if (ViewModel)
        {
            const UnrealMvvm_Impl::FViewModelPropertyReflection* MyProperty = UnrealMvvm_Impl::FViewModelRegistry::FindProperty(ViewModel->GetClass(), PropertyName);

            if (MyProperty)
            {
                MyProperty->CopyValueToMemory(ViewModel, OutValuePtr);
            }
        }
    }

    P_FINISH;
}