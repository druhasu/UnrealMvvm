// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/Impl/Property/ViewModelRegistry.h"
#include "Mvvm/BaseViewModel.h"
#include "Templates/SubclassOf.h"

namespace UnrealMvvm_Impl
{
    class FViewModelPropertyIterator
    {
    public:
        FViewModelPropertyIterator(TSubclassOf<UBaseViewModel> InViewModelClass, bool bMaintainOrder)
        {
            UClass* Class = InViewModelClass;
            while (Class && Class->IsChildOf<UBaseViewModel>())
            {
                TArray<FViewModelPropertyReflection>* List = FViewModelRegistry::ViewModelProperties.Find(Class);
                if (List)
                {
                    Lists.Add(List);
                }
                Class = Class->GetSuperClass();
            }

            // Correct order - base classes first
            if (bMaintainOrder)
            {
                Algo::Reverse(Lists);
            }
        }

        operator bool() const
        {
            return IsValid();
        }

        FViewModelPropertyIterator& operator++()
        {
            Advance();
            return *this;
        }

        const FViewModelPropertyReflection* operator->() const
        {
            check(IsValid());
            return &(*Lists[ListIndex])[Index];
        }

        const FViewModelPropertyReflection& operator*() const
        {
            check(IsValid());
            return (*Lists[ListIndex])[Index];
        }

        bool IsValid() const
        {
            return Lists.IsValidIndex(ListIndex) && Lists[ListIndex]->IsValidIndex(Index);
        }

        TArray<const FViewModelPropertyReflection*> ToArray()
        {
            TArray<const FViewModelPropertyReflection*> Result;
            for (; IsValid(); Advance())
            {
                Result.Add(&(*Lists[ListIndex])[Index]);
            }
            return Result;
        }

    private:
        void Advance()
        {
            if (!Lists.IsValidIndex(ListIndex))
            {
                return;
            }

            ++Index;

            if (!Lists[ListIndex]->IsValidIndex(Index))
            {
                ++ListIndex;
                Index = 0;
            }
        }

        int32 Index = 0;
        int32 ListIndex = 0;

        TArray<TArray<FViewModelPropertyReflection>*, TInlineAllocator<6>> Lists;
    };
}
