// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Mvvm/Impl/ViewModelRegistry.h"
#include "Mvvm/BaseViewModel.h"

using namespace UnrealMvvm_Impl;

TMap<UClass*, TArray<FViewModelPropertyReflection>> FViewModelRegistry::ViewModelProperties {};
TMap<UClass*, UClass*> FViewModelRegistry::ViewModelClasses {};
TArray<FViewModelRegistry::FUnprocessedPropertyEntry> FViewModelRegistry::UnprocessedProperties {};
TArray<FViewModelRegistry::FUnprocessedViewModelClassEntry> FViewModelRegistry::UnprocessedViewModelClasses {};

TArray<const FViewModelPropertyReflection*> FViewModelRegistry::GetProperties(UClass* InViewModelClass)
{
    ProcessProperties();

    TArray<const FViewModelPropertyReflection*> Result;

    if (InViewModelClass)
    {
        AppendProperties(Result, InViewModelClass);
    }

    return Result;
}

const FViewModelPropertyReflection* FViewModelRegistry::FindProperty(UClass* InViewModelClass, const FName& InPropertyName)
{
    ProcessProperties();

    if (InViewModelClass)
    {
        return FindPropertyInternal(InViewModelClass, InPropertyName);
    }
    
    return nullptr;
}

UClass* FViewModelRegistry::GetViewModelClass(UClass* ViewClass)
{
    ProcessClasses();

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

void FViewModelRegistry::ProcessProperties()
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
}

void FViewModelRegistry::ProcessClasses()
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
}

void FViewModelRegistry::AppendProperties(TArray<const FViewModelPropertyReflection*>& OutArray, UClass* InViewModelClass)
{
    // append properties of base class first
    UClass* SuperClass = InViewModelClass->GetSuperClass();
    if (SuperClass && SuperClass->IsChildOf<UBaseViewModel>())
    {
        AppendProperties(OutArray, SuperClass);
    }

    // append properties of requested class last
    TArray<FViewModelPropertyReflection>* ArrayPtr = ViewModelProperties.Find(InViewModelClass);
    if (ArrayPtr)
    {
        OutArray.Reserve(OutArray.Num() + ArrayPtr->Num());

        for (const FViewModelPropertyReflection& Item : *ArrayPtr)
        {
            OutArray.Add(&Item);
        }
    }
}

const FViewModelPropertyReflection* FViewModelRegistry::FindPropertyInternal(UClass* InViewModelClass, const FName& InPropertyName)
{
    // find properties of requested class
    TArray<FViewModelPropertyReflection>* ArrayPtr = ViewModelProperties.Find(InViewModelClass);

    if (ArrayPtr)
    {
        for(const FViewModelPropertyReflection& Item : *ArrayPtr)
        {
            if (Item.Property->GetName() == InPropertyName)
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