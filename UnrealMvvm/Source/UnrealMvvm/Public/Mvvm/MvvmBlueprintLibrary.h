// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "MvvmBlueprintLibrary.generated.h"

class UUserWidget;
class UBaseViewModel;
class AActor;

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

    /* Returns class of ViewModel that a View of ViewClass expects to use. Returns nullptr if no ViewModel is associated with the View */
    static TSubclassOf<UBaseViewModel> GetViewModelClass(UClass* ViewClass);

    /* Returns whether given Widget is in process of setting initial values from ViewModel */
    static bool IsInitializingProperty(UUserWidget* View)
    {
        return IsInitializingPropertyInWidget(View);
    }

    /* Returns whether given Actor is in process of setting initial values from ViewModel */
    static bool IsInitializingProperty(AActor* View)
    {
        return IsInitializingPropertyInActor(View);
    }

    /* Returns whether any View is in process of setting initial values from given ViewModel */
    static bool IsInitializingProperty(UBaseViewModel* ViewModel)
    {
        return IsInitializingPropertyFromViewModel(ViewModel);
    }

    /* Returns whether given Widget is in process of receiving new Value from any ViewModel property */
    static bool IsChangingProperty(UUserWidget* View)
    {
        return IsChangingPropertyInWidget(View);
    }

    /* Returns whether given Actor is in process of receiving new Value from any ViewModel property */
    static bool IsChangingProperty(AActor* View)
    {
        return IsChangingPropertyInActor(View);
    }

    /* Returns whether any View is in process of receiving new Value from given ViewModel */
    static bool IsChangingProperty(UBaseViewModel* ViewModel)
    {
        return IsChangingPropertyFromViewModel(ViewModel);
    }

private:
    // to access GetViewModelPropertyValueFrom... and SetViewModelPropertyValueTo... via GET_MEMBER_NAME_CHECKED
    friend class FViewModelPropertyNodeHelper;

    // to access GetViewModelFrom... and SetViewModelTo... via GET_MEMBER_NAME_CHECKED
    friend class UK2Node_ViewModelGetSet;

    // to access IsInitializingProperty... via GET_MEMBER_NAME_CHECKED
    friend class UK2Node_ViewModelPropertyChanged;

    UFUNCTION(BlueprintPure, CustomThunk, meta = (CustomStructureParam = "Value", BlueprintInternalUseOnly = "true"))
    static void GetViewModelPropertyValue(UBaseViewModel* ViewModel, FName PropertyName, int32& Value, bool& HasValue)
    {
        checkNoEntry();
    }

    UFUNCTION(BlueprintCallable, CustomThunk, meta = (CustomStructureParam = "Value", BlueprintInternalUseOnly = "true"))
    static void SetViewModelPropertyValue(UBaseViewModel* ViewModel, FName PropertyName, int32 Value, bool HasValue)
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

    /* Returns whether given Widget is in process of setting initial values from ViewModel */
    UFUNCTION(BlueprintPure, Category = "ViewModel", meta = (DefaultToSelf = "View"))
    static bool IsInitializingPropertyInWidget(UUserWidget* View);

    /* Returns whether given Actor is in process of setting initial values from ViewModel */
    UFUNCTION(BlueprintPure, Category = "ViewModel", meta = (DefaultToSelf = "View"))
    static bool IsInitializingPropertyInActor(AActor* View);

    /* Returns whether any View is in process of setting initial values from given ViewModel */
    UFUNCTION(BlueprintPure, Category = "ViewModel")
    static bool IsInitializingPropertyFromViewModel(UBaseViewModel* ViewModel);

    /* Returns whether given Widget is in process of receiving new Value from any ViewModel property */
    UFUNCTION(BlueprintPure, Category = "ViewModel", meta = (DefaultToSelf = "View"))
    static bool IsChangingPropertyInWidget(UUserWidget* View);

    /* Returns whether given Actor is in process of receiving new Value from any ViewModel property */
    UFUNCTION(BlueprintPure, Category = "ViewModel", meta = (DefaultToSelf = "View"))
    static bool IsChangingPropertyInActor(AActor* View);

    /* Returns whether any View is in process of receiving new Value from given ViewModel */
    UFUNCTION(BlueprintPure, Category = "ViewModel")
    static bool IsChangingPropertyFromViewModel(UBaseViewModel* ViewModel);

    DECLARE_FUNCTION(execGetViewModelPropertyValue);
    DECLARE_FUNCTION(execSetViewModelPropertyValue);

    template <typename TView, typename TViewComponent>
    static UBaseViewModel* GetViewModelInternal(TView* View);

    template <typename TView, typename TViewComponent>
    static void SetViewModelInternal(TView* View, UBaseViewModel* ViewModel);
};
