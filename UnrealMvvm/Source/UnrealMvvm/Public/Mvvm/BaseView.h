// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "Mvvm/BaseViewBind.h"
#include "Mvvm/ListenManager.h"
#include "BaseView.generated.h"

UCLASS()
class UNREALMVVM_API UBaseView : public UUserWidget, public FListenManager
{
    GENERATED_BODY()

protected:
    void NativeConstruct() override;
    void NativeDestruct() override;

    virtual void ListenEvents() {}

private:
    template<typename U, typename V>
    friend class TBaseView;

    UPROPERTY()
    class UBaseViewModel* ViewModel;
};
