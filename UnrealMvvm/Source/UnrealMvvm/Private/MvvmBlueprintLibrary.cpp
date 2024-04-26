// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Mvvm/MvvmBlueprintLibrary.h"
#include "Mvvm/Impl/ViewModelRegistry.h"
#include "Mvvm/Impl/BaseViewExtension.h"
#include "Mvvm/Impl/BaseViewComponent.h"
#include "Mvvm/BaseView.h"
#include "Mvvm/BaseViewModel.h"
#include "Misc/EngineVersionComparison.h"

#if !UE_VERSION_OLDER_THAN(5,4,0)
#include "Blueprint/BlueprintExceptionInfo.h"
#endif

UBaseViewModel* UMvvmBlueprintLibrary::GetViewModelFromWidget(UUserWidget* View)
{
    return GetViewModelInternal<UUserWidget, UBaseViewExtension>(View);
}

UBaseViewModel* UMvvmBlueprintLibrary::GetViewModelFromActor(AActor* View)
{
    return GetViewModelInternal<AActor, UBaseViewComponent>(View);
}

void UMvvmBlueprintLibrary::SetViewModelToWidget(UUserWidget* View, UBaseViewModel* ViewModel)
{
    SetViewModelInternal<UUserWidget, UBaseViewExtension>(View, ViewModel);
}

void UMvvmBlueprintLibrary::SetViewModelToActor(AActor* View, UBaseViewModel* ViewModel)
{
    SetViewModelInternal<AActor, UBaseViewComponent>(View, ViewModel);
}

DEFINE_FUNCTION(UMvvmBlueprintLibrary::execGetViewModelPropertyValue)
{
    execGetViewModelPropertyValueInternal(Context, Stack, RESULT_PARAM);
}

DEFINE_FUNCTION(UMvvmBlueprintLibrary::execGetViewModelPropertyValueFromWidget)
{
    execGetViewModelPropertyValueInternal<UUserWidget>(Context, Stack, RESULT_PARAM);
}

DEFINE_FUNCTION(UMvvmBlueprintLibrary::execSetViewModelPropertyValueToWidget)
{
    execSetViewModelPropertyValueInternal<UUserWidget>(Context, Stack, RESULT_PARAM);
}

DEFINE_FUNCTION(UMvvmBlueprintLibrary::execGetViewModelPropertyValueFromActor)
{
    execGetViewModelPropertyValueInternal<AActor>(Context, Stack, RESULT_PARAM);
}

DEFINE_FUNCTION(UMvvmBlueprintLibrary::execSetViewModelPropertyValueToActor)
{
    execSetViewModelPropertyValueInternal<AActor>(Context, Stack, RESULT_PARAM);
}

DEFINE_FUNCTION(UMvvmBlueprintLibrary::execSetViewModelPropertyValue)
{
    execSetViewModelPropertyValueInternal(Context, Stack, RESULT_PARAM);
}

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

    auto [ViewModel, MyProperty] = GetViewModelAndProperty(View, PropertyName);

    if (MyProperty)
    {
        MyProperty->GetOperations().GetValue(ViewModel, OutValuePtr, OutHasValueRef);
    }
    else
    {
        // althoguh we can safely continue execution of this function, we abort it for consistency with execSetViewModelPropertyValueInternal below
        FText ErrorMessage = FText::FormatOrdered(
            NSLOCTEXT("UnrealMvvm", "AccessInvalidProperty", "Cannot find ViewModel property '{0}' in ViewModel '{1}' of View '{2}'"),
            FText::FromName(PropertyName),
            FText::FromString(GetNameSafe(ViewModel)),
            FText::FromString(GetNameSafe(View))
        );

        FBlueprintExceptionInfo ExceptionInfo(EBlueprintExceptionType::AbortExecution, ErrorMessage);
        FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);
    }

    P_FINISH;
}

DEFINE_FUNCTION(UMvvmBlueprintLibrary::execGetViewModelPropertyValueInternal)
{
    P_GET_OBJECT(UBaseViewModel, ViewModel);
    P_GET_PROPERTY(FNameProperty, PropertyName);

    Stack.StepCompiledIn<FProperty>(nullptr);
    void* OutValuePtr = Stack.MostRecentPropertyAddress;

    P_GET_UBOOL_REF(OutHasValueRef);

    if (ViewModel)
    {
        if (const UnrealMvvm_Impl::FViewModelPropertyReflection* MyProperty = UnrealMvvm_Impl::FViewModelRegistry::FindProperty(ViewModel->GetClass(), PropertyName))
        {
            MyProperty->GetOperations().GetValue(ViewModel, OutValuePtr, OutHasValueRef);
        }
        else
        {
            // although we can safely continue execution of this function, we abort it for consistency with execSetViewModelPropertyValueInternal below
            FText ErrorMessage = FText::FormatOrdered(
                NSLOCTEXT("UnrealMvvm", "AccessInvalidProperty", "Cannot find ViewModel property '{0}' in ViewModel '{1}'"),
                FText::FromName(PropertyName),
                FText::FromString(GetNameSafe(ViewModel))
            );

            FBlueprintExceptionInfo ExceptionInfo(EBlueprintExceptionType::AbortExecution, ErrorMessage);
            FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);
        }
    }
    else
    {
        FBlueprintExceptionInfo ExceptionInfo(EBlueprintExceptionType::AccessViolation, NSLOCTEXT("UnrealMvvm", "AccessInvalidViewModel", "ViewModel was null."));
        FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);
    }

    P_FINISH;
}

DEFINE_FUNCTION(UMvvmBlueprintLibrary::execSetViewModelPropertyValueInternal)
{
    P_GET_OBJECT(UBaseViewModel, ViewModel);
    P_GET_PROPERTY(FNameProperty, PropertyName);

    if (ViewModel)
    {
        if (const UnrealMvvm_Impl::FViewModelPropertyReflection* MyProperty = UnrealMvvm_Impl::FViewModelRegistry::FindProperty(ViewModel->GetClass(), PropertyName))
        {
            void* StorageSpace = FMemory_Alloca(MyProperty->SizeOfValue);
            Stack.StepCompiledIn<FProperty>(StorageSpace);

            P_GET_UBOOL(HasValue);
        
            MyProperty->GetOperations().SetValue(ViewModel, StorageSpace, HasValue);
        }
        else
        {
            // we need to know exact size of this property's value to be able to allocate enough memory for it otherwise the script VM will crash.
            // so we abort the Blueprint execution, rather than whole process
            FText ErrorMessage = FText::FormatOrdered(
                NSLOCTEXT("UnrealMvvm", "AccessInvalidProperty", "Cannot find ViewModel property '{0}' in ViewModel '{1}'"),
                FText::FromName(PropertyName),
                FText::FromString(GetNameSafe(ViewModel))
            );

            FBlueprintExceptionInfo ExceptionInfo(EBlueprintExceptionType::AbortExecution, ErrorMessage);
            FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);
        }
    }
    else
    {
        FBlueprintExceptionInfo ExceptionInfo(EBlueprintExceptionType::AccessViolation, NSLOCTEXT("UnrealMvvm", "AccessInvalidViewModel", "ViewModel was null."));
        FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);
    }

    P_FINISH;
}

template <typename TView>
DEFINE_FUNCTION(UMvvmBlueprintLibrary::execSetViewModelPropertyValueInternal)
{
    P_GET_OBJECT(TView, View);
    P_GET_PROPERTY(FNameProperty, PropertyName);

    auto [ViewModel, MyProperty] = GetViewModelAndProperty(View, PropertyName);

    if (MyProperty)
    {
        void* StorageSpace = FMemory_Alloca(MyProperty->SizeOfValue);
        Stack.StepCompiledIn<FProperty>(StorageSpace);

        P_GET_UBOOL(HasValue);

        MyProperty->GetOperations().SetValue(ViewModel, StorageSpace, HasValue);
    }
    else
    {
        // we need to know exact size of this property's value to be able to allocate enough memory for it otherwise the script VM will crash.
        // so we abort the Blueprint execution, rather than whole process
        FText ErrorMessage = FText::FormatOrdered(
            NSLOCTEXT("UnrealMvvm", "AccessInvalidProperty", "Cannot find ViewModel property '{0}' in ViewModel '{1}' of View '{2}'"),
            FText::FromName(PropertyName),
            FText::FromString(GetNameSafe(ViewModel)),
            FText::FromString(GetNameSafe(View))
        );

        FBlueprintExceptionInfo ExceptionInfo(EBlueprintExceptionType::AbortExecution, ErrorMessage);
        FBlueprintCoreDelegates::ThrowScriptException(P_THIS, Stack, ExceptionInfo);
    }

    P_FINISH;
}

template <typename TView>
TTuple<UBaseViewModel*, const UnrealMvvm_Impl::FViewModelPropertyReflection*> UMvvmBlueprintLibrary::GetViewModelAndProperty(TView* View, FName PropertyName)
{
    if (View)
    {
        UBaseViewModel* ViewModel = GetViewModel(View);
        if (ViewModel)
        {
            return { ViewModel, UnrealMvvm_Impl::FViewModelRegistry::FindProperty(ViewModel->GetClass(), PropertyName) };
        }
    }

    return { nullptr, nullptr };
}
