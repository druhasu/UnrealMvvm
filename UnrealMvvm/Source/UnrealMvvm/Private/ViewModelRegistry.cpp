// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Mvvm/Impl/ViewModelRegistry.h"
#include "Mvvm/BaseViewModel.h"

using namespace UnrealMvvm_Impl;

TMap<UClass*, TArray<FViewModelPropertyReflection>> FViewModelRegistry::ViewModelProperties {};
TMap<UClass*, UClass*> FViewModelRegistry::ViewModelClasses {};
TArray<FViewModelRegistry::FUnprocessedPropertyEntry> FViewModelRegistry::UnprocessedProperties {};
TArray<FViewModelRegistry::FUnprocessedViewModelClassEntry> FViewModelRegistry::UnprocessedViewModelClasses {};

UClass* FViewModelRegistry::GetViewModelClass(UClass* ViewClass)
{
    // Process classes and add them into lookup tables
    if (UnprocessedViewModelClasses.Num())
    {
        for (auto& Entry : UnprocessedViewModelClasses)
        {
            ViewModelClasses.Add(Entry.GetViewClass(), Entry.GetViewModelClass());
        }

        UnprocessedViewModelClasses.Empty();
    }

    UClass* Needle = ViewClass;

    while (Needle)
    {
        UClass** ClassPtr = ViewModelClasses.Find(Needle);
        if (ClassPtr)
        {
            return *ClassPtr;
        }

        Needle = Needle->GetSuperClass();
    }

    return nullptr;
}

uint8 FViewModelRegistry::RegisterViewModelClass(FViewModelRegistry::ClassGetterPtr ViewClassGetter, FViewModelRegistry::ClassGetterPtr ViewModelClassGetter)
{
    FUnprocessedViewModelClassEntry& Entry = UnprocessedViewModelClasses.AddDefaulted_GetRef();
    Entry.GetViewClass = ViewClassGetter;
    Entry.GetViewModelClass = ViewModelClassGetter;

    return 1;
}

const TArray<FViewModelPropertyReflection>& FViewModelRegistry::GetProperties(UClass* InClass)
{
    // Process properties and add them into lookup tables
    if (UnprocessedProperties.Num())
    {
        for (auto& Property : UnprocessedProperties)
        {
            UClass* NewClass = Property.GetClass();
            TArray<FViewModelPropertyReflection>& NewArray = ViewModelProperties.FindOrAdd(NewClass);
            NewArray.Add(Property.Reflection);
        }

        UnprocessedProperties.Empty();
    }

    TArray<FViewModelPropertyReflection>* Array = ViewModelProperties.Find(InClass);

    if (Array)
    {
        return *Array;
    }

    static TArray<FViewModelPropertyReflection> Empty;
    return Empty;
}