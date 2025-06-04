// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Containers/Array.h"
#include "Templates/IsInvocable.h"
#include "Misc/EngineVersionComparison.h"

#ifndef UE_REQUIRES
#define UE_REQUIRES , TEMPLATE_REQUIRES
#endif

class UBaseViewModel;

namespace MvvmUtils
{
    /*
     * Synchronizes ViewModels collection to Models collection:
     *   - Creates and adds necessary amount of ViewModels
     *   - Removes excess ViewModels
     *   - Assigns each Model to respective ViewModel
     * 
     * Models can be represented as any type supported by ranged for loop: TArray, TArrayView, TMap, etc
     * 
     * ViewModels are created via NewObject<ViewModelType>()
     * Models are assigned via call to `ViewModel->SetModel(Model);`
     */
    template <typename TViewModel, typename TAllocator, typename TModels>
    void SyncViewModelCollection(TArray<TViewModel, TAllocator>& ViewModels, const TModels& Models)
    {
        SyncViewModelCollection(ViewModels, Models, [] { return NewObject<TPointedToType<TViewModel>>(); }, [](auto* ViewModel, auto& Model) { ViewModel->SetModel(Model); });
    }

    /*
     * Synchronizes ViewModels collection to Models collection:
     *   - Creates and adds necessary amount of ViewModels
     *   - Removes excess ViewModels
     *   - Assigns each Model to respective ViewModel
     * 
     * Models can be represented as any type supported by ranged for loop: TArray, TArrayView, TMap, etc
     *
     * ViewModels are created via provided Factory function. It has following signature: ViewModel* ()
     * Models are assigned via call to `ViewModel->SetModel(Model);`
     */
    template <typename TViewModel, typename TAllocator, typename TModels, typename TFactory UE_REQUIRES(TIsInvocable<TFactory>::Value)>
    void SyncViewModelCollection(TArray<TViewModel, TAllocator>& ViewModels, const TModels& Models, TFactory&& Factory)
    {
        SyncViewModelCollection(ViewModels, Models, Factory, [](auto* ViewModel, auto& Model) { ViewModel->SetModel(Model); });
    }

    /*
     * Synchronizes ViewModels collection to Models collection:
     *   - Creates and adds necessary amount of ViewModels
     *   - Removes excess ViewModels
     *   - Assigns each Model to respective ViewModel
     * 
     * Models can be represented as any type supported by ranged for loop: TArray, TArrayView, TMap, etc
     *
     * ViewModels are created via NewObject<ViewModelType>()
     * Models are assigned via call to provided Setter function. It has following signature: void (ViewModelType* ViewModel, const ModelType& Model)
     */
    template <typename TViewModel, typename TAllocator, typename TModels, typename TSetter UE_REQUIRES(!TIsInvocable<TSetter>::Value)>
    void SyncViewModelCollection(TArray<TViewModel, TAllocator>& ViewModels, const TModels& Models, TSetter&& Setter)
    {
        SyncViewModelCollection(ViewModels, Models, [] { return NewObject<TPointedToType<TViewModel>>(); }, Setter);
    }

    /*
     * Synchronizes ViewModels collection to Models collection:
     *   - Creates and adds necessary amount of ViewModels
     *   - Removes excess ViewModels
     *   - Assigns each Model to respective ViewModel
     * 
     * Models can be represented as any type supported by ranged for loop: TArray, TArrayView, TMap, etc
     *
     * ViewModels are created via provided Factory function. It has following signature: ViewModel* ()
     * Models are assigned via call to provided Setter function. It has following signature: void (ViewModelType* ViewModel, const ModelType& Model)
     */
    template <typename TViewModel, typename TAllocator, typename TModels, typename TFactory, typename TSetter>
    void SyncViewModelCollection(TArray<TViewModel, TAllocator>& ViewModels, const TModels& Models, TFactory&& Factory, TSetter&& Setter)
    {
        static_assert(TIsPointerOrObjectPtrToBaseOf<TViewModel, UBaseViewModel>::Value, "ViewModels array must contain pointers to or TObjectPtrs of UBaseViewModel");

        ViewModels.Reserve(Models.Num());

        while (ViewModels.Num() < Models.Num())
        {
            ViewModels.Add(Factory());
        }

        if (ViewModels.Num() > Models.Num())
        {
#if UE_VERSION_OLDER_THAN(5,5,0)
            ViewModels.RemoveAt(Models.Num(), ViewModels.Num() - Models.Num(), false);
#else
            ViewModels.RemoveAt(Models.Num(), ViewModels.Num() - Models.Num(), EAllowShrinking::No);
#endif
        }

        int32 Index = 0;
        for (auto& Model : Models)
        {
            Setter(ToRawPtr(ViewModels[Index]), Model);
            ++Index;
        }
    }
}
