// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Containers/ArrayView.h"

class FField;
class UClass;

namespace UnrealMvvm_Impl
{
    struct FViewModelPropertyReflection;

    struct UNREALMVVM_API FTokenStreamUtils
    {
        /* Adds all derived classes to a provided list */
        static void EnrichWithDerivedClasses(TArray<UClass*>& Classes);

        /* Sorts classes so Base ones goes before Derived ones */
        static void SortViewModelClasses(TArray<UClass*>& Classes);

        /* Generates FProperty objects from given ViewModel properties, adds them to given class and returns original pointer to first FProperty */ 
        static FField* AddPropertiesToClass(UClass* TargetClass, TArrayView<const FViewModelPropertyReflection> Properties);

        /* Destroys temporary properties keeping TSet and TMap properties and outputs them to provided array */
        static void CleanupProperties(UClass* TargetClass, FField* FirstFieldToKeep, TArray<FField*>& OutSavedProperties);
    };

}