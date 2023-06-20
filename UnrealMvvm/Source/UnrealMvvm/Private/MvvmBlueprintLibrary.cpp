// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Mvvm/MvvmBlueprintLibrary.h"
#include "Mvvm/Impl/ViewModelRegistry.h"
#include "Mvvm/Impl/BaseViewExtension.h"
#include "Mvvm/Impl/BaseViewComponent.h"
#include "Mvvm/BaseView.h"
#include "Mvvm/BaseViewModel.h"

template <typename TView, typename TViewComponent>
UBaseViewModel* UMvvmBlueprintLibrary::GetViewModelInternal(TView* View)
{
    if (!ensureAlways(View))
    {
        return nullptr;
    }

    TViewComponent* ViewComponent = TViewComponent::Get(View);
    if (!ViewComponent)
    {
        // this is a valid case, not every View must have it
        // even if it is a View, it may not have ViewComponent yet. in this case ViewModel is also not set
        return nullptr;
    }

    return ViewComponent->ViewModel;
}

template <typename TView, typename TViewComponent>
void UMvvmBlueprintLibrary::SetViewModelInternal(TView* View, UBaseViewModel* ViewModel)
{
    using namespace UnrealMvvm_Impl;

    if (!ensureAlways(View))
    {
        return;
    }

    UClass* ExpectedViewModelClass = FViewModelRegistry::GetViewModelClass(View->GetClass());
    if (!ExpectedViewModelClass)
    {
        // This Object is not a View, nothing to do here
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
        TViewComponent* Component = TViewComponent::Request(View);
        Component->SetViewModelInternal(ViewModel);
    }
}

template <typename TView>
DEFINE_FUNCTION(UMvvmBlueprintLibrary::execGetViewModelPropertyValueInternal)
{
    P_GET_OBJECT(TView, View);
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

template <typename TView>
DEFINE_FUNCTION(UMvvmBlueprintLibrary::execSetViewModelPropertyValueInternal)
{
    P_GET_OBJECT(TView, View);
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