// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/BaseView.h"
#include "TestBaseViewModel.h"
#include "TestBaseWidgetView.generated.h"

/* Test View based on BaseView class */
UCLASS()
class UTestBaseWidgetView : public UBaseView, public TBaseView<UTestBaseWidgetView, UTestBaseViewModel>
{
    GENERATED_BODY()

public:
    int32 MyValue = 0;
    ViewModelType* OldViewModel = nullptr;
    ViewModelType* NewViewModel = nullptr;

protected:
    void BindProperties() override
    {
        Bind(this, ViewModelType::IntValueProperty(), [this](const int32& InValue) { MyValue = InValue; });
    }

    void OnViewModelChanged(ViewModelType* InOldViewModel, ViewModelType* InNewViewModel) override
    {
        OldViewModel = InOldViewModel;
        NewViewModel = InNewViewModel;
    }
};

/* Same as UTestBaseWidgetView, but without registered bindings */
UCLASS()
class UTestBaseWidgetViewNoBind : public UBaseView, public TBaseView<UTestBaseWidgetViewNoBind, UTestBaseViewModel>
{
    GENERATED_BODY()
};

/* Test View based on UserWidget class */
UCLASS()
class UTestBaseWidgetViewPure : public UUserWidget, public TBaseView<UTestBaseWidgetViewPure, UTestBaseViewModel>
{
    GENERATED_BODY()

public:
    int32 MyValue = 0;
    ViewModelType* OldViewModel = nullptr;
    ViewModelType* NewViewModel = nullptr;

protected:
    void BindProperties() override
    {
        Bind(this, ViewModelType::IntValueProperty(), [this](const int32& InValue) { MyValue = InValue; });
    }

    void OnViewModelChanged(ViewModelType* InOldViewModel, ViewModelType* InNewViewModel) override
    {
        OldViewModel = InOldViewModel;
        NewViewModel = InNewViewModel;
    }
};

/* Same as UTestBaseWidgetViewPure, but without registered bindings */
UCLASS()
class UTestBaseWidgetViewPureNoBind : public UUserWidget, public TBaseView<UTestBaseWidgetViewPureNoBind, UTestBaseViewModel>
{
    GENERATED_BODY()
};

/* Test View base class for Blueprint-only view */
UCLASS()
class UTestBaseWidgetViewBlueprint : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintImplementableEvent)
    void SetValueToViewModel(int32 NewValue);

    UFUNCTION(BlueprintImplementableEvent)
    int32 GetValueFromViewModel();

    UPROPERTY(BlueprintReadWrite)
    int32 MyValue = 0;

    UPROPERTY(BlueprintReadWrite)
    UTestBaseViewModel* OldViewModel = nullptr;

    UPROPERTY(BlueprintReadWrite)
    UTestBaseViewModel* NewViewModel = nullptr;

    UPROPERTY(BlueprintReadWrite)
    UTestBaseViewModel* ViewModelFromGetter = nullptr;
};
