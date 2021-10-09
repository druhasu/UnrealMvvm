// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/BaseView.h"
#include "TestBaseViewModel.h"
#include "TestBaseView.generated.h"

UCLASS()
class UTestBaseView : public UBaseView, public TBaseView<UTestBaseView, UTestBaseViewModel>
{
    GENERATED_BODY()

public:
    int32 MyValue = 0;

protected:
    void BindProperties() override
    {
        Bind(this, ViewModelType::IntValueProperty(), [this](const int32& InValue) { MyValue = InValue; });
    }
};