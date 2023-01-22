// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Extensions/UserWidgetExtension.h"
#include "Mvvm/Impl/BindEntry.h" 
#include "BaseViewExtension.generated.h"

class UBaseViewModel;

UCLASS(Transient)
class UNREALMVVM_API UBaseViewExtension : public UUserWidgetExtension
{
    GENERATED_BODY()

public:
    void Construct() override;
    void Destruct() override;

    /* Name of UFunction to call when ViewModel changes */
    static const FName ViewModelChangedFunctionName;

private:
    template<typename U, typename V>
    friend class TBaseView;
    friend class UBaseView;
    friend class UMvvmBlueprintLibrary;
    friend class UBaseViewClassExtension;

    /* Returns Extension instance from a given widget. Creates new instance if not found */
    static UBaseViewExtension* Request(const UUserWidget* Widget);

    /* Returns existing Extension instance or nullptr if not found */
    static UBaseViewExtension* Get(const UUserWidget* Widget);

    /* Sets ViewModel and call required events */
    void SetViewModelInternal(UBaseViewModel* InViewModel);

    /* Fills BindEntries array */
    void PrepareBindings(UClass* ViewModelClass);

    /* Subscribes to ViewModel */
    void StartListening();

    /* Unsubscribes from ViewModel */
    void StopListening();

    /* Calls ViewModelChanged event, if it exist in blueprint class */
    void TryCallViewModelChanged(UBaseViewModel* OldViewModel, UBaseViewModel* NewViewModel);

    /* Called when ViewModel property changes */
    void OnPropertyChanged(const FViewModelPropertyBase* Property);

    bool HasBindings() const
    {
        return BindEntries.Num() > 0 && BindEntries[0].Property != nullptr;
    }

    UPROPERTY()
    UBaseViewModel* ViewModel;

    TArray< UnrealMvvm_Impl::FBindEntry > BindEntries;
    FDelegateHandle SubscriptionHandle;
};