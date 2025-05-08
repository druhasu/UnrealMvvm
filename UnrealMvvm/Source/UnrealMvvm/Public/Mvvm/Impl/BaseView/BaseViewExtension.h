// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Extensions/UserWidgetExtension.h"
#include "Mvvm/Impl/Binding/BindingWorker.h"
#include "Mvvm/Impl/BaseView/BaseViewComponentImpl.h"
#include "Blueprint/UserWidget.h"
#include "BaseViewExtension.generated.h"

UCLASS(Transient)
class UNREALMVVM_API UBaseViewExtension : public UUserWidgetExtension, public UnrealMvvm_Impl::TBaseViewComponentImpl<UBaseViewExtension>
{
    GENERATED_BODY()

public:
    void Construct() override;
    void Destruct() override;

    bool IsConstructed() const { return GetUserWidget()->IsConstructed(); }
    UObject* GetViewObject() const { return GetUserWidget(); }

private:
    /* There are a lot of friends here, but this class externals should not be made public */
    template<typename U, typename V>
    friend class TBaseView;
    friend class UBaseView;
    friend class UMvvmBlueprintLibrary;
    friend class UBaseViewClassExtension;
    template <typename TView>
    friend class UnrealMvvm_Impl::TBaseViewComponentImpl;
    template<typename O, typename V, typename U>
    friend class UnrealMvvm_Impl::TBaseViewImplWithComponent; // forward declared in BaseViewComponentImpl.h

    /* Returns Extension instance from a given widget. Creates new instance if not found */
    static UBaseViewExtension* Request(UUserWidget* Widget);

    /* Returns existing Extension instance or nullptr if not found */
    static UBaseViewExtension* Get(const UUserWidget* Widget);

    UPROPERTY()
    TObjectPtr<UBaseViewModel> ViewModel;

    UnrealMvvm_Impl::FBindingWorker BindingWorker;
};
