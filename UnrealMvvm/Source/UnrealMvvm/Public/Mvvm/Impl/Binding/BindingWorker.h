// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/Impl/Binding/BindingConfiguration.h"
#include "Mvvm/BaseViewModel.h"

namespace UnrealMvvm_Impl
{

    class UNREALMVVM_API FBindingWorker
    {
        using ThisClass = FBindingWorker;

    public:
        ~FBindingWorker()
        {
            StopListening();
        }

        void Init(UObject* InOwningView, const FBindingConfiguration& ConfigurationTemplate);

        template<typename THandler, typename... TArgs>
        THandler& AddBindingHandler(TArrayView<const FViewModelPropertyBase* const> PropertyPath, TArgs&&... Args)
        {
            return AddBindingHandlerImpl<const FViewModelPropertyBase* const, THandler, TArgs...>(PropertyPath, Forward<TArgs>(Args)...);
        }

        template<typename THandler, typename... TArgs>
        THandler& AddBindingHandler(TArrayView<const FName> PropertyPath, TArgs&&... Args)
        {
            return AddBindingHandlerImpl<const FName, THandler, TArgs...>(PropertyPath, Forward<TArgs>(Args)...);
        }

        UBaseViewModel* GetViewModel()
        {
            TArrayView<FResolvedViewModelEntry> ViewModelEntries = Bindings.GetViewModels();
            if (ViewModelEntries.Num() > 0)
            {
                // get Main ViewModel
                return ViewModelEntries[0].ViewModel;
            }
            return nullptr;
        }

        void SetViewModel(UBaseViewModel* InViewModel)
        {
            TArrayView<FResolvedViewModelEntry> ViewModelEntries = Bindings.GetViewModels();
            if (ViewModelEntries.Num() > 0)
            {
                // set Main ViewModel
                ViewModelEntries[0].ViewModel = InViewModel;
            }
        }

        void StartListening();

        void StopListening();

    private:
        void OnPropertyChanged(const FViewModelPropertyBase* Property, UBaseViewModel* ViewModel);

        void ProcessPropertyChange(UBaseViewModel* ViewModel, const FResolvedPropertyEntry& PropertyEntry);

        void PropagateChanges(const FResolvedViewModelEntry& ViewModelEntry);

        void Subscribe(UBaseViewModel* ViewModel)
        {
            ViewModel->Subscribe(UBaseViewModel::FPropertyChangedDelegate::FDelegate::CreateRaw(this, &ThisClass::OnPropertyChanged, ViewModel));
        }

        template<typename TPathEntry, typename THandler, typename... TArgs>
        THandler& AddBindingHandlerImpl(TArrayView<TPathEntry> PropertyPath, TArgs&&... Args)
        {
            check(PropertyPath.Num() > 0);
            check(Bindings.Data != nullptr);

            TArrayView<FResolvedViewModelEntry> ViewModelEntries = Bindings.GetViewModels();

            const FResolvedViewModelEntry* ViewModelEntry = &ViewModelEntries[0];
            FResolvedPropertyEntry* PropertyEntry = nullptr;

            for (int32 Index = 0, LastIndex = PropertyPath.Num() - 1; Index <= LastIndex; ++Index)
            {
                check(ViewModelEntry);

                const TPathEntry& Property = PropertyPath[Index];

                if (Index == LastIndex)
                {
                    // find first entry that does not have a handler
                    PropertyEntry = Bindings.GetProperties(*ViewModelEntry).FindByPredicate([&](const FResolvedPropertyEntry& Entry)
                    {
                        return Entry == Property && !Entry.bHasHandler;
                    });

                    check(PropertyEntry != nullptr);
                    PropertyEntry->EmplaceHandler<THandler>(Forward<TArgs>(Args)...);
                }
                else
                {
                    // find first entry that has next view model
                    PropertyEntry = Bindings.GetProperties(*ViewModelEntry).FindByPredicate([&](const FResolvedPropertyEntry& Entry)
                    {
                        return Entry == Property && Entry.NextViewModelIndex != INDEX_NONE;
                    });

                    check(PropertyEntry != nullptr);
                    ViewModelEntry = &ViewModelEntries[PropertyEntry->NextViewModelIndex];
                }
            }

            return *(THandler*)PropertyEntry->GetHandler();
        }

        UBaseViewModel* GetViewModelFromProperty(UBaseViewModel* ViewModel, const FViewModelPropertyBase* Property);

        UObject* OwningView;
        FBindingConfiguration Bindings;
    };

}
