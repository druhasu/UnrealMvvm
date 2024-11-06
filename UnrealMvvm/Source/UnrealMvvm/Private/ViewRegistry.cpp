// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Mvvm/Impl/BaseView/ViewRegistry.h"
#include "Mvvm/Impl/Binding/ViewModelDynamicBinding.h"
#include "Mvvm/Impl/Binding/BindingConfigurationBuilder.h"

namespace UnrealMvvm_Impl
{

#if WITH_EDITOR
FViewRegistry::FViewModelClassChanged FViewRegistry::ViewModelClassChanged;
#endif
TMap<TWeakObjectPtr<UClass>, UClass*> FViewRegistry::ViewModelClasses{};
TMap<UClass*, FViewRegistry::FViewModelSetterPtr> FViewRegistry::ViewModelSetters{};
TMap<UClass*, FViewRegistry::FBindingsCollectorPtr> FViewRegistry::BindingsCollectors{};
TMap<TWeakObjectPtr<UClass>, FBindingConfiguration> FViewRegistry::BindingConfigurations{};
FBindingConfigurationBuilder* FViewRegistry::CurrentConfigurationBuilder = nullptr;

template <typename TKey, typename TValue>
TValue* FindByClass(TMap<TKey, TValue*>& Map, UClass* ViewClass)
{
    UClass* Needle = ViewClass;

    while (Needle)
    {
        TValue** FoundPtr = Map.Find(Needle);
        if (FoundPtr)
        {
            return *FoundPtr;
        }

        Needle = Needle->GetSuperClass();
    }

    return nullptr;
}

template <typename TValue>
void ClearByKey(TMap<TWeakObjectPtr<UClass>, TValue>& Map)
{
    for (auto It = Map.CreateIterator(); It; ++It)
    {
        if (!It.Key().IsValid())
        {
            It.RemoveCurrent();
        }
    }
}

void FViewRegistry::ProcessPendingRegistrations()
{
    if (GIsInitialLoad)
    {
        // wait until UObject subsystem is loaded
        return;
    }

    // Process classes and add them into lookup tables
    auto& UnprocessedViewClasses = GetUnprocessedViewClasses();
    if (UnprocessedViewClasses.Num())
    {
        for (auto& Entry : UnprocessedViewClasses)
        {
            UClass* ViewClass = Entry.GetViewClass();
            UClass* ViewModelClass = Entry.GetViewModelClass();

            ViewModelClasses.Add(ViewClass, ViewModelClass);

            if (Entry.ViewModelSetter)
            {
                ViewModelSetters.Add(ViewClass, Entry.ViewModelSetter);
            }

            if (Entry.BindingsCollector)
            {
                BindingsCollectors.Add(ViewClass, Entry.BindingsCollector);
            }

            CreateBindingConfiguration(ViewClass, ViewModelClass);

#if WITH_EDITOR
            ViewModelClassChanged.Broadcast(ViewClass, ViewModelClass);
#endif
        }

        UnprocessedViewClasses.Empty();
    }
}

UClass* FViewRegistry::GetViewModelClass(UClass* ViewClass)
{
    // remove all entries where keys are no longer valid
    // we store BP classes there, so they may become unloaded or garbage collected
    ClearByKey(ViewModelClasses);

    return FindByClass(ViewModelClasses, ViewClass);
}

FViewRegistry::FViewModelSetterPtr FViewRegistry::GetViewModelSetter(UClass* ViewClass)
{
    return FindByClass(ViewModelSetters, ViewClass);
}

FViewRegistry::FBindingsCollectorPtr FViewRegistry::GetBindingsCollector(UClass* ViewClass)
{
    return FindByClass(BindingsCollectors, ViewClass);
}

const FBindingConfiguration* FViewRegistry::GetBindingConfiguration(UClass* ViewClass)
{
    // remove all entries where keys are no longer valid
    // we store BP classes there, so they may become unloaded or garbage collected
    ClearByKey(BindingConfigurations);

    return BindingConfigurations.Find(ViewClass);
}

uint8 FViewRegistry::RegisterViewClass(FClassGetterPtr ViewClassGetter, FClassGetterPtr ViewModelClassGetter, FViewModelSetterPtr ViewModelSetter, FBindingsCollectorPtr BindingsCollector)
{
    TArray<FUnprocessedViewClassEntry>& UnprocessedEntries = GetUnprocessedViewClasses();

    FUnprocessedViewClassEntry& Entry = UnprocessedEntries.AddDefaulted_GetRef();
    Entry.GetViewClass = ViewClassGetter;
    Entry.GetViewModelClass = ViewModelClassGetter;
    Entry.ViewModelSetter = ViewModelSetter;
    Entry.BindingsCollector = BindingsCollector;

    return 1;
}

void FViewRegistry::RegisterViewClass(UClass* ViewClass, UClass* ViewModelClass)
{
    check(ViewClass);
    check(ViewModelClass);

    ViewModelClasses.Emplace(ViewClass, ViewModelClass);

    CreateBindingConfiguration(ViewClass, ViewModelClass);

#if WITH_EDITOR
    ViewModelClassChanged.Broadcast(ViewClass, ViewModelClass);
#endif
}

#if WITH_EDITOR
void FViewRegistry::UnregisterViewClass(UClass* ViewClass)
{
    check(ViewClass);

    ViewModelClasses.Remove(ViewClass);
    ViewModelClassChanged.Broadcast(ViewClass, nullptr);
}
#endif

bool FViewRegistry::RecordPropertyPath(TArrayView<const FViewModelPropertyBase* const> PropertyPath)
{
    if (CurrentConfigurationBuilder == nullptr)
    {
        return false;
    }

    CurrentConfigurationBuilder->AddBinding(PropertyPath);
    return true;
}

void FViewRegistry::CreateBindingConfiguration(UClass* ViewClass, UClass* ViewModelClass)
{
    // init builder
    FBindingConfigurationBuilder Builder(ViewModelClass);

    // collect native bindings
    {
        TGuardValue<FBindingConfigurationBuilder*> Guard(CurrentConfigurationBuilder, &Builder);

        FBindingsCollectorPtr BindingsCollector = FindByClass(BindingsCollectors, ViewClass);
        if (BindingsCollector != nullptr)
        {
            UObject* CDO = ViewClass->GetDefaultObject(false);
            if (CDO == nullptr)
            {
                // this function may be called during Blueprint compilation, when CDO is not created yet
                // in this case we can skip entirely
                return;
            }

            BindingsCollector(*CDO);
        }
    }

    // collect blueprint bindings
    UViewModelDynamicBinding* ViewModelDynamicBinding = static_cast<UViewModelDynamicBinding*>(UBlueprintGeneratedClass::GetDynamicBindingObject(ViewClass, UViewModelDynamicBinding::StaticClass()));
    if (ViewModelDynamicBinding != nullptr)
    {
        for (const FBlueprintBindingEntry& Binding : ViewModelDynamicBinding->BlueprintBindings)
        {
            Builder.AddBinding(Binding.PropertyPath);
        }
    }

    BindingConfigurations.Emplace(ViewClass, Builder.Build());
}

TArray<FViewRegistry::FUnprocessedViewClassEntry>& FViewRegistry::GetUnprocessedViewClasses()
{
    static TArray<FUnprocessedViewClassEntry> Result;
    return Result;
}

}
