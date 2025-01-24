// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Containers/Array.h"

class UObject;
class UBaseViewModel;
class FViewModelPropertyBase;

namespace UnrealMvvm_Impl
{

    class FViewChangeTracker
    {
    public:
        static bool IsInitializing(UObject* View)
        {
            return CurrentInitializations.ContainsByPredicate([&](const FInitEntry& Entry)
            {
                return Entry.View == View;
            });
        }

        static bool IsInitializing(UBaseViewModel* ViewModel)
        {
            return CurrentInitializations.ContainsByPredicate([&](const FInitEntry& Entry)
            {
                return Entry.ViewModel == ViewModel;
            });
        }

        static bool IsChanging(UObject* View)
        {
            return CurrentChanges.ContainsByPredicate([&](const FChangeEntry& Entry)
            {
                return Entry.View == View;
            });
        }

        static bool IsChanging(UBaseViewModel* ViewModel)
        {
            return CurrentChanges.ContainsByPredicate([&](const FChangeEntry& Entry)
            {
                return Entry.ViewModel == ViewModel;
            });
        }

        static bool IsChanging(const FViewModelPropertyBase* Property)
        {
            return CurrentChanges.ContainsByPredicate([&](const FChangeEntry& Entry)
            {
                return Entry.Property == Property;
            });
        }

    private:
        friend struct FViewInitializationScope;
        friend struct FViewChangeScope;

        static void PushInitialization(UObject* View, UBaseViewModel* ViewModel)
        {
            CurrentInitializations.Emplace(View, ViewModel);
        }

        static void PopInitialization()
        {
            check(CurrentInitializations.Num() > 0);
            CurrentInitializations.RemoveAt(CurrentInitializations.Num() - 1, 1, false);
        }

        static void PushChange(UObject* View, UBaseViewModel* ViewModel, const FViewModelPropertyBase* Property)
        {
            CurrentChanges.Emplace(View, ViewModel, Property);
        }

        static void PopChange()
        {
            check(CurrentChanges.Num() > 0);
            CurrentChanges.RemoveAt(CurrentChanges.Num() - 1, 1, false);
        }

        struct FInitEntry
        {
            UObject* View;
            UBaseViewModel* ViewModel;
        };

        struct FChangeEntry
        {
            UObject* View;
            UBaseViewModel* ViewModel;
            const FViewModelPropertyBase* Property;
        };

        static inline TArray<FInitEntry> CurrentInitializations;
        static inline TArray<FChangeEntry> CurrentChanges;
    };

    struct FViewInitializationScope
    {
        FViewInitializationScope(UObject* View, UBaseViewModel* ViewModel)
        {
            FViewChangeTracker::PushInitialization(View, ViewModel);
        }

        ~FViewInitializationScope()
        {
            FViewChangeTracker::PopInitialization();
        }
    };

    struct FViewChangeScope
    {
        FViewChangeScope(UObject* View, UBaseViewModel* ViewModel, const FViewModelPropertyBase* Property)
        {
            FViewChangeTracker::PushChange(View, ViewModel, Property);
        }

        ~FViewChangeScope()
        {
            FViewChangeTracker::PopChange();
        }
    };

}
