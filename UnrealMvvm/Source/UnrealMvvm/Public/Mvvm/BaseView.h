// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "Mvvm/BaseViewBind.h"
#include "Mvvm/ListenManager.h"
#include "BaseView.generated.h"

/*
* Base class for View.
* Actual view must inherit from this class and from TBaseView.
*/
UCLASS()
class UNREALMVVM_API UBaseView : public UUserWidget, public FListenManager
{
    GENERATED_BODY()

public:
    UBaseView();

    /* Returns pointer to current BaseViewModel */
    class UBaseViewModel* GetUntypedViewModel() const { return GetExtension<UBaseViewExtension>()->ViewModel; }

protected:
    void NativeConstruct() override;
    void NativeDestruct() override;

    virtual void ListenEvents() {}
};
