#include "Mvvm/Impl/TokenStreamUtils.h"
#include "Mvvm/Impl/ViewModelPropertyReflection.h"

void UnrealMvvm_Impl::FTokenStreamUtils::SortViewModelClasses(TArray<UClass*>& Classes)
{
    Algo::Sort(Classes, [](UClass* Left, UClass* Right)
    {
        return Right->IsChildOf(Left);
    });
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
}