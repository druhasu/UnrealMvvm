// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Mvvm/Impl/Property/ViewModelPropertyNamesCache.h"
#include "Mvvm/ViewModelProperty.h"

using namespace UnrealMvvm_Impl;

TMap<const FViewModelPropertyBase*, FViewModelPropertyNamesCache::FNamesEntry> FViewModelPropertyNamesCache::Cache{};

const FViewModelPropertyNamesCache::FNamesEntry& FViewModelPropertyNamesCache::GetEntry(const FViewModelPropertyBase* Property)
{
    FNamesEntry* EntryPtr = Cache.Find(Property);

    if (EntryPtr == nullptr)
    {
        EntryPtr = &Cache.Add(Property);

        EntryPtr->PropertyName = Property->Name;
        EntryPtr->FunctionName = MakeCallbackName(EntryPtr->PropertyName);
    }

    return *EntryPtr;
}

FName FViewModelPropertyNamesCache::MakeCallbackName(const FName& PropertyName)
{
    return *FString::Printf(TEXT("OnVM_%s"), *PropertyName.ToString());
}
