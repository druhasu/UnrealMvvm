// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Mvvm/Impl/Binding/BindingWorker.h"
#include "Mvvm/Impl/Binding/IPropertyChangeHandler.h"
#include "Mvvm/Impl/BaseView/ViewChangeTracker.h"
#include "Mvvm/Impl/Property/ViewModelRegistry.h"

namespace UnrealMvvm_Impl
{

void FBindingWorker::Init(UObject* InOwningView, const FBindingConfiguration& ConfigurationTemplate)
{
    OwningView = InOwningView;
    Bindings = ConfigurationTemplate;

    for (FResolvedViewModelEntry& ViewModelEntry : Bindings.GetViewModels())
    {
        // clear ViewModel Class received from "template" version
        ViewModelEntry.ViewModelClass = nullptr;
    }
}

void FBindingWorker::StartListening()
{
    if (GetViewModel() == nullptr || Bindings.HasSubscription())
    {
        return;
    }

    Bindings.SetHasSubscription(true);

    TArrayView<FResolvedViewModelEntry> ViewModelEntries = Bindings.GetViewModels();

    // subscribe to existing ViewModels
    for (FResolvedViewModelEntry& ViewModelEntry : ViewModelEntries)
    {
        if (ViewModelEntry.ViewModel == nullptr)
        {
            // TODO: check if there are properties that explicitly handle "no value" and invoke their handlers

            // we don't have instance of ViewModel in this entry, skip all of its properties
            continue;
        }

        Subscribe(ViewModelEntry.ViewModel);

        TArrayView<FResolvedPropertyEntry> PropertyEntries = Bindings.GetProperties(ViewModelEntry);
        for (const FResolvedPropertyEntry& PropertyEntry : PropertyEntries)
        {
            if (PropertyEntry.NextViewModelIndex != INDEX_NONE)
            {
                UBaseViewModel* ViewModel = GetViewModelFromProperty(ViewModelEntry.ViewModel, PropertyEntry.Property);
                ViewModelEntries[PropertyEntry.NextViewModelIndex].ViewModel = ViewModel;
            }

            // property may have no handler if it is used only inside "property path" binding
            if (IPropertyChangeHandler* Handler = PropertyEntry.GetHandler())
            {
                Handler->Invoke(ViewModelEntry.ViewModel, PropertyEntry.Property);
            }
        }
    }
}

void FBindingWorker::StopListening()
{
    if (!Bindings.HasSubscription())
    {
        return;
    }

    Bindings.SetHasSubscription(false);

    TArrayView<FResolvedViewModelEntry> ViewModelEntries = Bindings.GetViewModels();

    // unsubscribe from existing ViewModels
    for(int32 Index = 0; Index < ViewModelEntries.Num(); ++Index)
    {
        FResolvedViewModelEntry& ViewModelEntry = ViewModelEntries[Index];
        if (ViewModelEntry.ViewModel != nullptr)
        {
            ViewModelEntry.ViewModel->Unsubscribe(this);

            // clear all entries except the first one
            if (Index > 0)
            {
                ViewModelEntry.ViewModel = nullptr;
            }
        }
    }

    // TODO: check if there are properties that explicitly handle "no value" and invoke their handlers
}

void FBindingWorker::OnPropertyChanged(const FViewModelPropertyBase* Property, UBaseViewModel* ViewModel)
{
    TArrayView<FResolvedViewModelEntry> ViewModelEntries = Bindings.GetViewModels();

    FResolvedViewModelEntry* ViewModelEntry = ViewModelEntries.FindByKey(ViewModel);
    check(ViewModelEntry); // should never be null

    TArrayView<FResolvedPropertyEntry> PropertyEntries = Bindings.GetProperties(*ViewModelEntry);
    for (const FResolvedPropertyEntry& PropertyEntry : PropertyEntries)
    {
        if (PropertyEntry == Property)
        {
            ProcessPropertyChange(ViewModel, PropertyEntry);
        }
    }
}

void FBindingWorker::ProcessPropertyChange(UBaseViewModel* ViewModel, const FResolvedPropertyEntry& PropertyEntry)
{
    if (PropertyEntry.NextViewModelIndex != INDEX_NONE)
    {
        FResolvedViewModelEntry& ViewModelEntry = Bindings.GetViewModels()[PropertyEntry.NextViewModelIndex];

        UBaseViewModel* CurrentViewModel = GetViewModelFromProperty(ViewModel, PropertyEntry.Property);
        UBaseViewModel* CachedViewModel = ViewModelEntry.ViewModel;

        if (CurrentViewModel != CachedViewModel)
        {
            ViewModelEntry.ViewModel = CurrentViewModel;

            if (CachedViewModel != nullptr)
            {
                CachedViewModel->Unsubscribe(this);
            }

            if (CurrentViewModel != nullptr)
            {
                Subscribe(CurrentViewModel);
            }

            // propagate changes of all properties
            PropagateChanges(ViewModelEntry);
        }
    }

    // property may have no handler if it is used only inside "property path" binding
    if (IPropertyChangeHandler* Handler = PropertyEntry.GetHandler())
    {
        UnrealMvvm_Impl::FViewChangeScope Scope(OwningView, ViewModel, PropertyEntry.Property);
        Handler->Invoke(ViewModel, PropertyEntry.Property);
    }
}

void FBindingWorker::PropagateChanges(const FResolvedViewModelEntry& ViewModelEntry)
{
    TArrayView<FResolvedPropertyEntry> PropertyEntries = Bindings.GetProperties(ViewModelEntry);
    for (const FResolvedPropertyEntry& PropertyEntry : PropertyEntries)
    {
        ProcessPropertyChange(ViewModelEntry.ViewModel, PropertyEntry);
    }
}

UBaseViewModel* FBindingWorker::GetViewModelFromProperty(UBaseViewModel* ViewModel, const FViewModelPropertyBase* Property)
{
    if (ViewModel == nullptr)
    {
        return nullptr;
    }

    using namespace UnrealMvvm_Impl;
    const FViewModelPropertyReflection* Reflection = FViewModelRegistry::FindProperty(ViewModel->GetClass(), Property->GetName());

    if (Reflection)
    {
        UBaseViewModel* OutViewModel = nullptr;
        bool OutHasValue = false;
        Reflection->GetOperations().GetValue(ViewModel, &OutViewModel, OutHasValue);

        if (OutHasValue)
        {
            return OutViewModel;
        }
    }

    return nullptr;
}

}
