// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "WidgetBlueprintExtension.h"
#include "BaseViewBlueprintExtension.generated.h"

UCLASS()
class UBaseViewBlueprintExtension : public UWidgetBlueprintExtension
{
    GENERATED_BODY()

public:
    void HandleBeginCompilation(FWidgetBlueprintCompilerContext& InCreationContext) override;
    void HandleFinishCompilingClass(UWidgetBlueprintGeneratedClass* Class) override;

    UPROPERTY()
    UClass* ViewModelClass;

private:
    FWidgetBlueprintCompilerContext* Context;
};