// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Mvvm/MvvmBlueprintLibrary.h"
#include "Mvvm/Impl/BaseView/BaseViewExtension.h"
#include "Mvvm/Impl/BaseView/BaseViewComponent.h"
#include "Mvvm/Impl/BaseView/ViewChangeTracker.h"
#include "Mvvm/Impl/BaseView/ViewRegistry.h"
#include "Mvvm/Impl/Property/ViewModelRegistry.h"
#include "Mvvm/BaseViewModel.h"
#include "Misc/EngineVersionComparison.h"

#if !UE_VERSION_OLDER_THAN(5,4,0)
#include "Blueprint/BlueprintExceptionInfo.h"
#endif

TSubclassOf<UBaseViewModel> UMvvmBlueprintLibrary::GetViewModelClass(UClass* ViewClass)
{
    return UnrealMvvm_Impl::FViewRegistry::GetViewModelClass(ViewClass);;
}

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

bool UMvvmBlueprintLibrary::IsInitializingPropertyInWidget(UUserWidget* View)
{
    return UnrealMvvm_Impl::FViewChangeTracker::IsInitializing(View);
}

bool UMvvmBlueprintLibrary::IsInitializingPropertyInActor(AActor* View)
{
    return UnrealMvvm_Impl::FViewChangeTracker::IsInitializing(View);
}

bool UMvvmBlueprintLibrary::IsInitializingPropertyFromViewModel(UBaseViewModel* ViewModel)
{
    return UnrealMvvm_Impl::FViewChangeTracker::IsInitializing(ViewModel);
}

bool UMvvmBlueprintLibrary::IsChangingPropertyInWidget(UUserWidget* View)
{
    return UnrealMvvm_Impl::FViewChangeTracker::IsChanging(View);
}

bool UMvvmBlueprintLibrary::IsChangingPropertyInActor(AActor* View)
{
    return UnrealMvvm_Impl::FViewChangeTracker::IsChanging(View);
}

bool UMvvmBlueprintLibrary::IsChangingPropertyFromViewModel(UBaseViewModel* ViewModel)
{
    return UnrealMvvm_Impl::FViewChangeTracker::IsChanging(ViewModel);
}

DEFINE_FUNCTION(UMvvmBlueprintLibrary::execGetViewModelPropertyValue)
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

DEFINE_FUNCTION(UMvvmBlueprintLibrary::execSetViewModelPropertyValue)
{
    P_GET_OBJECT(UBaseViewModel, ViewModel);
    P_GET_PROPERTY(FNameProperty, PropertyName);

    if (ViewModel)
    {
        if (const UnrealMvvm_Impl::FViewModelPropertyReflection* MyProperty = UnrealMvvm_Impl::FViewModelRegistry::FindProperty(ViewModel->GetClass(), PropertyName))
        {
            // allocate enough space for holding property value
            void* StorageSpace = FMemory_Alloca(MyProperty->SizeOfValue);
            FProperty* NextProperty = nullptr;
            UScriptStruct* NextStruct = nullptr;

            // remember current Code pointer, because we may need to modify it and restore later
            uint8* SavedCode = Stack.Code;

            switch (*Stack.Code)
            {
                case EX_LocalVariable:
                case EX_InstanceVariable:
                case EX_DefaultVariable:
                {
                    // increase Code to emulate single Step, but without actually executing anything
                    Stack.Code++;

                    // read property from Stack
                    NextProperty = Stack.ReadPropertyUnchecked();
                    check(NextProperty != nullptr);

                    // initialize memory for storing property value
                    NextProperty->InitializeValue(StorageSpace);
                }
                break;

                case EX_StructConst:
                {
                    // increase Code to emulate single Step, but without actually executing anything
                    Stack.Code++;

                    // read struct from Stack
                    NextStruct = (UScriptStruct*)Stack.ReadObject();
                    check(NextStruct != nullptr);

                    // initialize memory for storing struct value
                    NextStruct->InitializeStruct(StorageSpace);
                }
                break;

                default:
                    FMemory::Memzero(StorageSpace, MyProperty->SizeOfValue);
            }

            // restore previous pointer
            Stack.Code = SavedCode;

            // read the value from Blueprint graph
            Stack.StepCompiledIn<FProperty>(StorageSpace);

            P_GET_UBOOL(HasValue);

            // store the value into ViewModel
            MyProperty->GetOperations().SetValue(ViewModel, StorageSpace, HasValue);

            if (NextProperty != nullptr)
            {
                // properly destroy the value
                NextProperty->DestroyValue(StorageSpace);
            }
            else if (NextStruct != nullptr)
            {
                // properly destroy the value
                NextStruct->DestroyStruct(StorageSpace);
            }
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

    UClass* ExpectedViewModelClass = FViewRegistry::GetViewModelClass(View->GetClass());
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
    FViewRegistry::FViewModelSetterPtr CustomSetter = FViewRegistry::GetViewModelSetter(View->GetClass());
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
