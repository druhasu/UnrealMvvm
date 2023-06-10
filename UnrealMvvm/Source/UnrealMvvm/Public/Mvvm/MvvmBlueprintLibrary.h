// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "MvvmBlueprintLibrary.generated.h"

class UUserWidget;
class UBaseViewModel;
class UBaseView;

UCLASS()
class UNREALMVVM_API UMvvmBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /* Returns ViewModel from a View widget. Supports Blueprint-only Views as well */
    UFUNCTION(BlueprintPure, Category = "ViewModel", meta = (BlueprintInternalUseOnly = "true"))
    static UBaseViewModel* GetViewModel(UUserWidget* View);

    /* Sets ViewModel to a View widget. Supports Blueprint-only Views as well */
    UFUNCTION(BlueprintCallable, Category = "ViewModel", meta = (BlueprintInternalUseOnly = "true"))
    static void SetViewModel(UUserWidget* View, UBaseViewModel* ViewModel);

private:
    // to access GetViewModelPropertyValue and SetViewModelPropertyValue via GET_MEMBER_NAME_CHECKED
    friend class FViewModelPropertyNodeHelper;

    UFUNCTION(BlueprintPure, CustomThunk, meta = (CustomStructureParam = "Value", BlueprintInternalUseOnly = "true"))
    static void GetViewModelPropertyValue(UUserWidget* View, FName PropertyName, int32& Value, bool& HasValue);

    UFUNCTION(BlueprintCallable, CustomThunk, meta = (CustomStructureParam = "Value", BlueprintInternalUseOnly = "true"))
    static void SetViewModelPropertyValue(UUserWidget* View, FName PropertyName, int32 Value, bool HasValue);

    DECLARE_FUNCTION(execGetViewModelPropertyValue);
    DECLARE_FUNCTION(execSetViewModelPropertyValue);
};