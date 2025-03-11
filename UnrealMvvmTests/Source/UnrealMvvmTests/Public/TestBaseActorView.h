// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/BaseView.h"
#include "TestBaseViewModel.h"
#include "TestBaseActorView.generated.h"

/* Test View based on Actor class */
UCLASS()
class ATestBaseActorViewPure : public AActor, public TBaseView<ATestBaseActorViewPure, UTestBaseViewModel>
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

/* Same as ATestBaseActorViewPure, but without registered bindings */
UCLASS()
class ATestBaseActorViewPureNoBind : public AActor, public TBaseView<ATestBaseActorViewPureNoBind, UTestBaseViewModel>
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite)
    int32 MyValue = 0;

    UPROPERTY(BlueprintReadWrite)
    UTestBaseViewModel* OldViewModel = nullptr;

    UPROPERTY(BlueprintReadWrite)
    UTestBaseViewModel* NewViewModel = nullptr;
};

/* Test View base class for Blueprint-only view */
UCLASS()
class ATestBaseActorViewBlueprint : public AActor
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
    UTestBaseViewModel* OldViewModel = nullptr;

    UPROPERTY(BlueprintReadWrite)
    UTestBaseViewModel* NewViewModel = nullptr;

    UPROPERTY(BlueprintReadWrite)
    UTestBaseViewModel* ViewModelFromGetter = nullptr;
};

/* Test View base class to record IsInitial results from PropertyChanged events */
UCLASS()
class ATestBaseActorViewIsInitialTracker : public AActor
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite)
    TArray<bool> IsInitialResults;
};
