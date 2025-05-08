// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "Mvvm/Impl/Binding/BindingWorker.h"
#include "Mvvm/Impl/BaseView/BaseViewComponentImpl.h"
#include "BaseViewComponent.generated.h"

UCLASS(meta = (BlueprintSpawnableComponent))
class UNREALMVVM_API UBaseViewComponent : public UActorComponent, public UnrealMvvm_Impl::TBaseViewComponentImpl<UBaseViewComponent>
{
	GENERATED_BODY()

public:
    void BeginPlay() override;
    void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    bool IsConstructed() const { return GetOwner()->HasActorBegunPlay(); }
    UObject* GetViewObject() const { return GetOwner(); }

private:
    /* There are a lot of friends here, but this class externals should not be made public */
    template<typename U, typename V>
    friend class TBaseView;
    friend class UMvvmBlueprintLibrary;
    template <typename TView>
    friend class UnrealMvvm_Impl::TBaseViewComponentImpl;
    template<typename O, typename V, typename U>
    friend class UnrealMvvm_Impl::TBaseViewImplWithComponent; // forward declared in BaseViewComponentImpl.h

    /* Returns Component instance from a given actor. Creates new instance if not found */
    static UBaseViewComponent* Request(AActor* Actor);

    /* Returns existing Component instance or nullptr if not found */
    static UBaseViewComponent* Get(const AActor* Actor);

    UPROPERTY()
    TObjectPtr<UBaseViewModel> ViewModel;

    UnrealMvvm_Impl::FBindingWorker BindingWorker;
};
