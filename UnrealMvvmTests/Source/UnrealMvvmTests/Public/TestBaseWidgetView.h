// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/BaseView.h"
#include "TestBaseViewModel.h"
#include "TestBaseWidgetView.generated.h"

/* Test View based on UserWidget class */
UCLASS()
class UTestBaseWidgetViewPure : public UUserWidget, public TBaseView<UTestBaseWidgetViewPure, UTestBaseViewModel>
{
    GENERATED_BODY()

public:
    int32 MyValue = 0;
    ViewModelType* OldViewModel = nullptr;
    ViewModelType* NewViewModel = nullptr;

    TFunction<void(int32)> IntValueChangedCallback;

protected:
    void BindProperties() override
    {
        Bind(this, ViewModelType::IntValueProperty(), [this](const int32& InValue)
        {
            MyValue = InValue;
            if (IntValueChangedCallback)
            {
                IntValueChangedCallback(InValue);
            }
        });
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

public:
    UPROPERTY(BlueprintReadWrite)
    int32 MyValue = 0;

    UPROPERTY(BlueprintReadWrite)
    TObjectPtr<UTestBaseViewModel> OldViewModel = nullptr;

    UPROPERTY(BlueprintReadWrite)
    TObjectPtr<UTestBaseViewModel> NewViewModel = nullptr;
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
    void SetValueToViewModelConstant();

    UFUNCTION(BlueprintImplementableEvent)
    void SetValueToViewModelStruct();

    UFUNCTION(BlueprintImplementableEvent)
    int32 GetValueFromViewModel();

    UPROPERTY(BlueprintReadWrite)
    int32 MyValue = 0;

    UPROPERTY(BlueprintReadWrite)
    float MyFloatValue = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 MyDerivedValue = 0;

    UPROPERTY(BlueprintReadWrite)
    TObjectPtr<UTestBaseViewModel> OldViewModel = nullptr;

    UPROPERTY(BlueprintReadWrite)
    TObjectPtr<UTestBaseViewModel> NewViewModel = nullptr;

    UPROPERTY(BlueprintReadWrite)
    TObjectPtr<UTestBaseViewModel> ViewModelFromGetter = nullptr;
};

/* Test View base class to record IsInitial results from PropertyChanged events */
UCLASS()
class UTestBaseWidgetViewIsInitialTracker : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite)
    TArray<bool> IsInitialResults;
};
