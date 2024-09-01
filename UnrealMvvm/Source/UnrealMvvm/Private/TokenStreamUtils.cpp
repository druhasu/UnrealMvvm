// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Mvvm/Impl/Property/TokenStreamUtils.h"
#include "Mvvm/Impl/Property/ViewModelPropertyReflection.h"
#include "Algo/TopologicalSort.h"

void UnrealMvvm_Impl::FTokenStreamUtils::EnrichWithDerivedClasses(TArray<UClass*>& Classes)
{
    TArray<UClass*> DerivedClasses;
    DerivedClasses.Reserve(64); // start with big enough buffer, because we will collect all derived classes here

    // collect all derived classes in the same array
    for (UClass* ViewModelClass : Classes)
    {
        GetDerivedClasses(ViewModelClass, DerivedClasses, true);
    }

    // add DerivedClasses back to our Classes array
    for (UClass* DerivedClass : DerivedClasses)
    {
        // make sure we don't have duplicates here
        Classes.AddUnique(DerivedClass);
    }
}

void UnrealMvvm_Impl::FTokenStreamUtils::SortViewModelClasses(TArray<UClass*>& Classes)
{
    // TopologicalSort may return incorrect results in case we have class hierarachy A -> B -> C, but only A and C present in Classes
    // but we always call EnrichWithDerivedClasses before sorting, so this situation never happens actually
    Algo::TopologicalSort(Classes, [](UClass* C) { return TArray<UClass*, TFixedAllocator<1>>{ C->GetSuperClass() }; });
}

FField* UnrealMvvm_Impl::FTokenStreamUtils::AddPropertiesToClass(UClass* TargetClass, TArrayView<const FViewModelPropertyReflection> Properties)
{
    // Save relevant properties to temporary array
    TArray<const FViewModelPropertyReflection*, TInlineAllocator<32>> TempProperties;
    for (const auto& PropertyReflection : Properties)
    {
        TempProperties.Emplace(&PropertyReflection);
    }

    // UClass properties are added to front of a linked list.
    // If we iterate them later, they will be in reverse order.
    // Generated TokenStream will then also have tokens in reverse order.
    // Reading values from memory back-to-front is slow (cache misses and all of that).
    // So we reverse our properties list beforehand, so generated TokenStream will have cache friendly order of iteration.
    Algo::Reverse(TempProperties);

    // Save original pointer to first property, so we can restore it later
    FField* FirstOriginalField = TargetClass->ChildProperties;

    // Create FProperty objects and add them to ViewModelClass
    for (const FViewModelPropertyReflection* Property : TempProperties)
    {
        Property->GetOperations().AddClassProperty(TargetClass);
    }

    // Link class to populate cached data inside newly created FProperties
    TargetClass->StaticLink();

    return FirstOriginalField;
}

void UnrealMvvm_Impl::FTokenStreamUtils::CleanupProperties(UClass* TargetClass, FField* FirstFieldToKeep, TArray<FField*>& OutSavedProperties)
{
    // delete all unrelevant properties
    FField* CurrentField = TargetClass->ChildProperties;

    while (CurrentField && CurrentField != FirstFieldToKeep)
    {
        FField* NextField = CurrentField->Next;

        // TMap and TSet properties are used during garbage collection, so we must keep them alive
        const bool bKeepField = CurrentField->IsA<FMapProperty>() || CurrentField->IsA<FSetProperty>();
        if (bKeepField)
        {
            OutSavedProperties.Add(CurrentField);
        }
        else
        {
            delete CurrentField;
        }

        CurrentField = NextField;
    }

    TargetClass->ChildProperties = FirstFieldToKeep;

    // Link class to restore PropertyLink and others
    TargetClass->StaticLink();
}
