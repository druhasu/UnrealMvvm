// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Mvvm/Impl/ViewModelRegistry.h"
#include "Mvvm/BaseViewModel.h"
#include "Mvvm/Impl/ViewModelPropertyIterator.h"
#include "Mvvm/Impl/TokenStreamUtils.h"
#include "Algo/Compare.h"

namespace UnrealMvvm_Impl
{

#if WITH_EDITOR
FViewModelRegistry::FViewModelClassChanged FViewModelRegistry::ViewClassChanged;
#endif
TMap<UClass*, TArray<FViewModelPropertyReflection>> FViewModelRegistry::ViewModelProperties{};
TMap<TWeakObjectPtr<UClass>, UClass*> FViewModelRegistry::ViewModelClasses{};
TMap<UClass*, FViewModelRegistry::FViewModelSetterPtr> FViewModelRegistry::ViewModelSetters{};
TArray<FField*> FViewModelRegistry::PropertiesToKeep{};

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

const FViewModelPropertyReflection* FViewModelRegistry::FindProperty(UClass* InViewModelClass, const FName& InPropertyName)
{
    if (InViewModelClass)
    {
        return FindPropertyInternal(InViewModelClass, InPropertyName);
    }

    return nullptr;
}

UClass* FViewModelRegistry::GetViewModelClass(UClass* ViewClass)
{
    // remove all entries where keys are no longer valid
    // we store BP classes there, so they may become unloaded or garbage collected
    for (auto It = ViewModelClasses.CreateIterator(); It; ++It)
    {
        if (!It.Key().IsValid())
        {
            It.RemoveCurrent();
        }
    }

    return FindByClass(ViewModelClasses, ViewClass);
}

FViewModelRegistry::FViewModelSetterPtr FViewModelRegistry::GetViewModelSetter(UClass* ViewClass)
{
    return FindByClass(ViewModelSetters, ViewClass);
}

uint8 FViewModelRegistry::RegisterViewClass(FViewModelRegistry::FClassGetterPtr ViewClassGetter, FViewModelRegistry::FClassGetterPtr ViewModelClassGetter, FViewModelRegistry::FViewModelSetterPtr ViewModelSetter)
{
    FUnprocessedViewModelClassEntry& Entry = GetUnprocessedViewModelClasses().AddDefaulted_GetRef();
    Entry.GetViewClass = ViewClassGetter;
    Entry.GetViewModelClass = ViewModelClassGetter;
    Entry.ViewModelSetter = ViewModelSetter;

    return 1;
}

void FViewModelRegistry::RegisterViewClass(UClass* ViewClass, UClass* ViewModelClass)
{
    check(ViewClass);
    check(ViewModelClass);

    ViewModelClasses.Emplace(ViewClass, ViewModelClass);
#if WITH_EDITOR
    ViewClassChanged.Broadcast(ViewClass, ViewModelClass);
#endif
}

#if WITH_EDITOR
void FViewModelRegistry::UnregisterViewClass(UClass* ViewClass)
{
    check(ViewClass);

    ViewModelClasses.Remove(ViewClass);
    ViewClassChanged.Broadcast(ViewClass, nullptr);
}
#endif

void FViewModelRegistry::ProcessPendingRegistrations()
{
    if (GIsInitialLoad)
    {
        // wait until UObject subsystem is loaded
        return;
    }

    // Process properties and add them into lookup tables
    TArray<UClass*> NewlyAddedViewModels;

    if (GetUnprocessedProperties().Num())
    {
        for (auto& Property : GetUnprocessedProperties())
        {
            UClass* NewClass = Property.GetClass();

            TArray<FViewModelPropertyReflection>* NewArray = ViewModelProperties.Find(NewClass);
            if (!NewArray)
            {
                NewArray = &ViewModelProperties.Emplace(NewClass);
                NewlyAddedViewModels.Emplace(NewClass);
            }

            NewArray->Add(Property.Reflection);
        }

        GetUnprocessedProperties().Empty();
    }

    // Add all derived classes of NewlyAddedViewModels to the list
    // Some derived classes may have no properties, but their token streams must still be adjusted to account for tokens of base classes
    // We need to add them manually because we won't know about them otherwise
    FTokenStreamUtils::EnrichWithDerivedClasses(NewlyAddedViewModels);

    // Sort ViewModels so base classes are located before derived ones
    // This way we guarantee that base classes' token streams will be generated first
    // If they are in different modules, Unreal will handle module load ordering
    // In monolithic mode all viewmodels are processed at once, like in a single module
    FTokenStreamUtils::SortViewModelClasses(NewlyAddedViewModels);

    // Patch all new ViewModel classes, so their properties will be processed by GC
    for (UClass* ViewModelClass : NewlyAddedViewModels)
    {
        GenerateReferenceTokenStream(ViewModelClass);
    }

    // Process classes and add them into lookup tables
    if (GetUnprocessedViewModelClasses().Num())
    {
        for (auto& Entry : GetUnprocessedViewModelClasses())
        {
            UClass* ViewClass = Entry.GetViewClass();
            UClass* ViewModelClass = Entry.GetViewModelClass();

            ViewModelClasses.Add(ViewClass, ViewModelClass);

            if (Entry.ViewModelSetter)
            {
                ViewModelSetters.Add(ViewClass, Entry.ViewModelSetter);
            }

#if WITH_EDITOR
            ViewClassChanged.Broadcast(ViewClass, ViewModelClass);
#endif
        }

        GetUnprocessedViewModelClasses().Empty();
    }
}

void FViewModelRegistry::DeleteKeptProperties()
{
    for (FField* Field : PropertiesToKeep)
    {
        delete Field;
    }

    PropertiesToKeep.Reset();
}

const FViewModelPropertyReflection* FViewModelRegistry::FindPropertyInternal(UClass* InViewModelClass, const FName& InPropertyName)
{
    // find properties of requested class
    TArray<FViewModelPropertyReflection>* ArrayPtr = ViewModelProperties.Find(InViewModelClass);

    if (ArrayPtr)
    {
        for (const FViewModelPropertyReflection& Item : *ArrayPtr)
        {
            if (Item.GetProperty()->GetName() == InPropertyName)
            {
                return &Item;
            }
        }
    }

    // if not found - look in a super class
    UClass* SuperClass = InViewModelClass->GetSuperClass();
    if (SuperClass && SuperClass->IsChildOf<UBaseViewModel>())
    {
        return FindPropertyInternal(SuperClass, InPropertyName);
    }

    return nullptr;
}

void FViewModelRegistry::GenerateReferenceTokenStream(UClass* ViewModelClass)
{
    // We take only properties of current class, because AssembleTokenStream merges tokens with Super class
    const TArray<FViewModelPropertyReflection>* Properties = ViewModelProperties.Find(ViewModelClass);

    if (Properties == nullptr)
    {
        // this ViewModel class does not have its own properties, just update token stream to include tokens from parent class
        ViewModelClass->AssembleReferenceTokenStream(true);
        return;
    }

    // Generate FProperties and add them to ViewModel class
    FField* FirstOriginalField = FTokenStreamUtils::AddPropertiesToClass(ViewModelClass, MakeArrayView(*Properties));

    // Create token stream that includes our new FProperties
    ViewModelClass->AssembleReferenceTokenStream(true);

    // delete all unrelevant properties and restore original list pointer
    FTokenStreamUtils::CleanupProperties(ViewModelClass, FirstOriginalField, PropertiesToKeep);
}

TArray<FViewModelRegistry::FUnprocessedPropertyEntry>& FViewModelRegistry::GetUnprocessedProperties()
{
    static TArray<FUnprocessedPropertyEntry> Result;
    return Result;
}

TArray<FViewModelRegistry::FUnprocessedViewModelClassEntry>& FViewModelRegistry::GetUnprocessedViewModelClasses()
{
    static TArray<FUnprocessedViewModelClassEntry> Result;
    return Result;
}

}