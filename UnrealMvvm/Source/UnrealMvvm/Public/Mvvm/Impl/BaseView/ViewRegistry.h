// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/Impl/Binding/BindingConfiguration.h"

class UClass;
class UObject;
class UBaseViewModel;
class FViewModelPropertyBase;

namespace UnrealMvvm_Impl
{
    class FBindingConfigurationBuilder;

    class UNREALMVVM_API FViewRegistry
    {
    public:
        using FClassGetterPtr = UClass * (*)();
        using FViewModelSetterPtr = void (*)(UObject&, UBaseViewModel*);
        using FBindingsCollectorPtr = void (*)(UObject&);

        static void ProcessPendingRegistrations();

        static UClass* GetViewModelClass(UClass* ViewClass);

        static FViewModelSetterPtr GetViewModelSetter(UClass* ViewClass);
        static FBindingsCollectorPtr GetBindingsCollector(UClass* ViewClass);
        static const FBindingConfiguration* GetBindingConfiguration(UClass* ViewClass);

        static uint8 RegisterViewClass(FClassGetterPtr ViewClassGetter, FClassGetterPtr ViewModelClassGetter, FViewModelSetterPtr ViewModelSetter, FBindingsCollectorPtr BindingsCollector);
        static void RegisterViewClass(UClass* ViewClass, UClass* ViewModelClass);

#if WITH_EDITOR
        static void UnregisterViewClass(UClass* ViewClass);
#endif

        static bool RecordPropertyPath(TArrayView<const FViewModelPropertyBase* const> PropertyPath);

#if WITH_EDITOR
        DECLARE_MULTICAST_DELEGATE_TwoParams(FViewModelClassChanged, UClass* /*ViewClass*/, UClass* /*ViewModelClass*/);
        static FViewModelClassChanged ViewModelClassChanged;
#endif

    private:

        struct FUnprocessedViewClassEntry
        {
            FClassGetterPtr GetViewClass;
            FClassGetterPtr GetViewModelClass;
            FViewModelSetterPtr ViewModelSetter;
            FBindingsCollectorPtr BindingsCollector;
        };

        static void CreateBindingConfiguration(UClass* ViewClass, UClass* ViewModelClass);

        // List of view model classes that were not yet added to lookup table
        static TArray<FUnprocessedViewClassEntry>& GetUnprocessedViewClasses();

        // Map of <ViewClass, ViewModelClass>
        static TMap<TWeakObjectPtr<UClass>, UClass*> ViewModelClasses;

        // Map of <ViewClass, Setter Function>
        static TMap<UClass*, FViewModelSetterPtr> ViewModelSetters;

        // Map of <ViewClass, Collector Function>
        static TMap<UClass*, FBindingsCollectorPtr> BindingsCollectors;

        // Map of <ViewClass, Resolved Binding Configuration>
        static TMap<TWeakObjectPtr<UClass>, FBindingConfiguration> BindingConfigurations;

        // Current list of Native handlers that we collect
        static FBindingConfigurationBuilder* CurrentConfigurationBuilder;
    };
}
