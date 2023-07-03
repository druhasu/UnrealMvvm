// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "MvvmBlueprintLibrary.generated.h"

class UUserWidget;
class UBaseViewModel;
class UBaseView;
class UBaseViewExtension;
class AActor;
class UBaseViewComponent;

namespace UnrealMvvm_Impl
{
    struct FViewModelPropertyReflection;
}

UCLASS()
class UNREALMVVM_API UMvvmBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /* Returns ViewModel from a View widget. Supports Blueprint-only Views as well */
    static UBaseViewModel* GetViewModel(UUserWidget* View)
    {
        return GetViewModelFromWidget(View);
    }

    /* Returns ViewModel from a View actor. Supports Blueprint-only Views as well */
    static UBaseViewModel* GetViewModel(AActor* View)
    {
        return GetViewModelFromActor(View);
    }

    /* Sets ViewModel to a View widget. Supports Blueprint-only Views as well */
    static void SetViewModel(UUserWidget* View, UBaseViewModel* ViewModel)
    {
        SetViewModelToWidget(View, ViewModel);
    }

    /* Sets ViewModel to a View actor. Supports Blueprint-only Views as well */
    static void SetViewModel(AActor* View, UBaseViewModel* ViewModel)
    {
        SetViewModelToActor(View, ViewModel);
    }

private:
    // to access GetViewModelPropertyValueFrom... and SetViewModelPropertyValueTo... via GET_MEMBER_NAME_CHECKED
    friend class FViewModelPropertyNodeHelper;

    // to access GetViewModelFrom... and SetViewModelTo... via GET_MEMBER_NAME_CHECKED
    friend class UK2Node_ViewModelGetSet;

    UFUNCTION(BlueprintPure, CustomThunk, meta = (CustomStructureParam = "Value", BlueprintInternalUseOnly = "true"))
    static void GetViewModelPropertyValueFromWidget(UUserWidget* View, FName PropertyName, int32& Value, bool& HasValue)
    {
        checkNoEntry();
    }

    UFUNCTION(BlueprintCallable, CustomThunk, meta = (CustomStructureParam = "Value", BlueprintInternalUseOnly = "true"))
    static void SetViewModelPropertyValueToWidget(UUserWidget* View, FName PropertyName, int32 Value, bool HasValue)
    {
        checkNoEntry();
    }

    UFUNCTION(BlueprintPure, CustomThunk, meta = (CustomStructureParam = "Value", BlueprintInternalUseOnly = "true"))
    static void GetViewModelPropertyValueFromActor(AActor* View, FName PropertyName, int32& Value, bool& HasValue)
    {
        checkNoEntry();
    }

    UFUNCTION(BlueprintCallable, CustomThunk, meta = (CustomStructureParam = "Value", BlueprintInternalUseOnly = "true"))
    static void SetViewModelPropertyValueToActor(AActor* View, FName PropertyName, int32 Value, bool HasValue)
    {
        checkNoEntry();
    }

    UFUNCTION(BlueprintPure, Category = "ViewModel", meta = (BlueprintInternalUseOnly = "true"))
    static UBaseViewModel* GetViewModelFromWidget(UUserWidget* View);

    UFUNCTION(BlueprintPure, Category = "ViewModel", meta = (BlueprintInternalUseOnly = "true"))
    static UBaseViewModel* GetViewModelFromActor(AActor* View);

    UFUNCTION(BlueprintCallable, Category = "ViewModel", meta = (BlueprintInternalUseOnly = "true"))
    static void SetViewModelToWidget(UUserWidget* View, UBaseViewModel* ViewModel);

    UFUNCTION(BlueprintCallable, Category = "ViewModel", meta = (BlueprintInternalUseOnly = "true"))
    static void SetViewModelToActor(AActor* View, UBaseViewModel* ViewModel);

    DECLARE_FUNCTION(execGetViewModelPropertyValueFromWidget);
    DECLARE_FUNCTION(execSetViewModelPropertyValueToWidget);
    DECLARE_FUNCTION(execGetViewModelPropertyValueFromActor);
    DECLARE_FUNCTION(execSetViewModelPropertyValueToActor);

    template <typename TView, typename TViewComponent>
    static UBaseViewModel* GetViewModelInternal(TView* View);

    template <typename TView, typename TViewComponent>
    static void SetViewModelInternal(TView* View, UBaseViewModel* ViewModel);

    template<typename TView>
    DECLARE_FUNCTION(execGetViewModelPropertyValueInternal);

    template<typename TView>
    DECLARE_FUNCTION(execSetViewModelPropertyValueInternal);

    template <typename TView>
    static TTuple<UBaseViewModel*, const UnrealMvvm_Impl::FViewModelPropertyReflection*> GetViewModelAndProperty(TView* View, FName PropertyName);
};