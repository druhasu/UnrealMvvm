// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Mvvm/Impl/Property/ViewModelRegistry.h"
#include "Mvvm/BaseViewModel.h"
#include "Mvvm/Impl/Property/TokenStreamUtils.h"

namespace UnrealMvvm_Impl
{

TMap<UClass*, TArray<FViewModelPropertyReflection>> FViewModelRegistry::ViewModelProperties{};
TArray<FField*> FViewModelRegistry::PropertiesToKeep{};

const FViewModelPropertyReflection* FViewModelRegistry::FindProperty(UClass* InViewModelClass, const FName& InPropertyName)
{
    if (InViewModelClass)
    {
        return FindPropertyInternal(InViewModelClass, InPropertyName);
    }

    return nullptr;
}

void FViewModelRegistry::ProcessPendingRegistrations()
{
    if (GIsInitialLoad)
    {
        // wait until UObject subsystem is loaded
        return;
    }

    // Process properties and add them into lookup tables
    TArray<UClass*> NewlyAddedViewModels;

    auto& UnprocessedProperties = GetUnprocessedProperties();
    if (UnprocessedProperties.Num())
    {
        for (auto& Property : UnprocessedProperties)
        {
            UClass* NewClass = Property.GetClass();

            TArray<FViewModelPropertyReflection>* NewArray = ViewModelProperties.Find(NewClass);
            if (!NewArray)
            {
                NewArray = &ViewModelProperties.Emplace(NewClass);
                NewlyAddedViewModels.Emplace(NewClass);
            }

            // We need to check that we don't have same property already registered
            // It is possible if same ViewModel is referenced from different modules (.dll) leading to multiple instantiations of registrator template
            const bool bPropertyAlreadyRegistered = NewArray->ContainsByPredicate([&](const FViewModelPropertyReflection& ExistingReflection)
            {
                return ExistingReflection.GetProperty() == Property.Reflection.GetProperty();
            });

            if (!bPropertyAlreadyRegistered)
            {
                NewArray->Add(Property.Reflection);
            }
        }

        UnprocessedProperties.Empty();
    }

    if (NewlyAddedViewModels.Num() > 0)
    {
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

}
