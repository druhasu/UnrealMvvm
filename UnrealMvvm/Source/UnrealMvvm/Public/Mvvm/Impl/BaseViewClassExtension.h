// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Extensions/WidgetBlueprintGeneratedClassExtension.h"
#include "BaseViewClassExtension.generated.h"

UCLASS()
class UNREALMVVM_API UBaseViewClassExtension : public UWidgetBlueprintGeneratedClassExtension
{
    GENERATED_BODY()

public:
    void Initialize(UUserWidget* UserWidget) override;
    void PreConstruct(UUserWidget* UserWidget, bool IsDesignTime) override;
    void PostLoad() override;

    UPROPERTY()
    UClass* ViewModelClass;
};