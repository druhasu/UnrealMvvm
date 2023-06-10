// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/BaseView.h"
#include "TestBaseViewModel.h"
#include "TestBaseView.generated.h"

/* Test View based on BaseView class */
UCLASS()
class UTestBaseView : public UBaseView, public TBaseView<UTestBaseView, UTestBaseViewModel>
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

/* Same as UTestBaseView, but without registered bindings */
UCLASS()
class UTestBaseViewNoBind : public UBaseView, public TBaseView<UTestBaseViewNoBind, UTestBaseViewModel>
{
    GENERATED_BODY()
};

/* Test View based on UserWidget class */
UCLASS()
class UTestBaseViewPure : public UUserWidget, public TBaseView<UTestBaseViewPure, UTestBaseViewModel>
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

/* Same as UTestBaseViewPure, but without registered bindings */
UCLASS()
class UTestBaseViewPureNoBind : public UUserWidget, public TBaseView<UTestBaseViewPureNoBind, UTestBaseViewModel>
{
    GENERATED_BODY()
};

/* Test View base class for Blueprint-only view */
UCLASS()
class UTestBaseViewBlueprint : public UUserWidget
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