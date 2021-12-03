// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Mvvm/BaseViewModel.h"
#include "Mvvm/BaseView.h"
#include "MvvmBlueprintLibrary.generated.h"

UCLASS()
class UNREALMVVM_API UMvvmBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, CustomThunk, meta = (CustomStructureParam = "Value", BlueprintInternalUseOnly = "true"))
    static void GetViewModelPropertyValue(class UBaseView* View, FName PropertyName, int32& Value);

    DECLARE_FUNCTION(execGetViewModelPropertyValue);
};