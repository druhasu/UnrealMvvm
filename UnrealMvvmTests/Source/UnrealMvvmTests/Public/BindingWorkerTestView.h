// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "Mvvm/BaseView.h"
#include "BindingWorkerTestViewModel.h"

#include "BindingWorkerTestView.generated.h"

UCLASS()
class UBindingWorkerTestView : public UUserWidget, public TBaseView<UBindingWorkerTestView, UBindingWorkerViewModel_Root>
{
    GENERATED_BODY()

public:
    int32 RootValue = 0;
    int32 FirstChildValue = 0;
    int32 SecondChildValue = 0;

protected:
    void BindProperties()
    {
        Bind(this, Path(ViewModelType::IntValueProperty()), [this](int32 V) { RootValue = V; });
        Bind(this, Path(ViewModelType::ChildProperty(), UBindingWorkerViewModel_FirstChild::IntValueProperty()), [this](int32 V) { FirstChildValue = V; });
        Bind(this, Path(ViewModelType::ChildProperty(), UBindingWorkerViewModel_FirstChild::ChildProperty(), UBindingWorkerViewModel_SecondChild::IntValueProperty()), [this](int32 V) { SecondChildValue = V; });
    }
};
