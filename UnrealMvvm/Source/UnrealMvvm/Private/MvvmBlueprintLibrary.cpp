// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Mvvm/MvvmBlueprintLibrary.h"
#include "Mvvm/Impl/ViewModelRegistry.h"
#include "Mvvm/Impl/BaseViewExtension.h"
#include "Mvvm/BaseView.h"
#include "Mvvm/BaseViewModel.h"

UBaseViewModel* UMvvmBlueprintLibrary::GetViewModel(UUserWidget* View)
{
    if (!ensureAlways(View))
    {
        return nullptr;
    }

    UBaseViewExtension* Extension = View->GetExtension<UBaseViewExtension>();
    if (!Extension)
    {
        // this is a valid case, not every widget must have it
        // even if it is a View, it may not have Extension yet. in this case ViewModel is also not set
        return nullptr;
    }

    return Extension->ViewModel;
}

void UMvvmBlueprintLibrary::SetViewModel(UUserWidget* View, UBaseViewModel* ViewModel)
{
    using namespace UnrealMvvm_Impl;

    if (!ensureAlways(View))
    {
        return;
    }

    UClass* ExpectedViewModelClass = FViewModelRegistry::GetViewModelClass(View->GetClass());
    if (!ExpectedViewModelClass)
    {
        // This widget is not a View, nothing to do here
        return;
    }

    if (ViewModel)
    {
        // Make sure provided ViewModel is of a correct class or else View will crash
        if (!ensureAlways(ViewModel->IsA(ExpectedViewModelClass)))
        {
            return;
        }
    }

    // TBaseView needs additional things done when setting ViewModel. So it will have custom setter registered
    FViewModelRegistry::FViewModelSetterPtr CustomSetter = FViewModelRegistry::GetViewModelSetter(View->GetClass());
    if (CustomSetter)
    {
        CustomSetter(*View, ViewModel);
    }
    else
    {
        UBaseViewExtension* Extension = UBaseViewExtension::Request(View);
        Extension->SetViewModelInternal(ViewModel);
    }
}

void UMvvmBlueprintLibrary::GetViewModelPropertyValue(UUserWidget* View, FName PropertyName, int32& Value, bool& HasValue)
{
    checkNoEntry();
}

void UMvvmBlueprintLibrary::SetViewModelPropertyValue(UUserWidget* View, FName PropertyName, int32 Value, bool HasValue)
{
    checkNoEntry();
}

DEFINE_FUNCTION(UMvvmBlueprintLibrary::execGetViewModelPropertyValue)
{
    P_GET_OBJECT(UUserWidget, View);
    P_GET_PROPERTY(FNameProperty, PropertyName);

    Stack.StepCompiledIn<FProperty>(nullptr);
    void* OutValuePtr = Stack.MostRecentPropertyAddress;

    P_GET_UBOOL_REF(OutHasValueRef);

    if (View)
    {
        UBaseViewModel* ViewModel = GetViewModel(View);
        if (ViewModel)
        {
            const UnrealMvvm_Impl::FViewModelPropertyReflection* MyProperty = UnrealMvvm_Impl::FViewModelRegistry::FindProperty(ViewModel->GetClass(), PropertyName);

            if (MyProperty)
            {
                MyProperty->GetOperations().GetValue(ViewModel, OutValuePtr, OutHasValueRef);
            }
        }
    }

    P_FINISH;
}

DEFINE_FUNCTION(UMvvmBlueprintLibrary::execSetViewModelPropertyValue)
{
    P_GET_OBJECT(UUserWidget, View);
    P_GET_PROPERTY(FNameProperty, PropertyName);

    Stack.StepCompiledIn<FProperty>(nullptr);
    void* OutValuePtr = Stack.MostRecentPropertyAddress;

    P_GET_UBOOL(HasValue);

    if (View)
    {
        UBaseViewModel* ViewModel = GetViewModel(View);
        if (ViewModel)
        {
            const UnrealMvvm_Impl::FViewModelPropertyReflection* MyProperty = UnrealMvvm_Impl::FViewModelRegistry::FindProperty(ViewModel->GetClass(), PropertyName);

            if (MyProperty)
            {
                MyProperty->GetOperations().SetValue(ViewModel, OutValuePtr, HasValue);
            }
        }
    }

    P_FINISH;
}