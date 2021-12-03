// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

class FViewModelPropertyBase;

namespace UnrealMvvm_Impl
{

    class UNREALMVVM_API FViewModelPropertyNamesCache
    {
    public:
        static FName GetPropertyName(const FViewModelPropertyBase* Property)
        {
            return GetEntry(Property).PropertyName;
        }

        static FName GetPropertyCallbackName(const FViewModelPropertyBase* Property)
        {
            return GetEntry(Property).FunctionName;
        }

        static FName MakeCallbackName(const FName& PropertyName);

    private:
        struct FNamesEntry
        {
            FName PropertyName;
            FName FunctionName;
        };

        static const FNamesEntry& GetEntry(const FViewModelPropertyBase*);

        static TMap<const FViewModelPropertyBase*, FNamesEntry> Cache;
    };

}