// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Containers/Array.h"

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
    void SyncViewModelCollection(TArray<TViewModel*, TAllocator>& ViewModels, const TModels& Models)
    {
        SyncViewModelCollection(ViewModels, Models, [] { return NewObject<TViewModel>(); }, [](auto* ViewModel, auto& Model) { ViewModel->SetModel(Model); });
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
    template <typename TViewModel, typename TAllocator, typename TModels, typename TFactory, TEMPLATE_REQUIRES(TIsInvocable<TFactory>::Value)>
    void SyncViewModelCollection(TArray<TViewModel*, TAllocator>& ViewModels, const TModels& Models, TFactory&& Factory)
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
    template <typename TViewModel, typename TAllocator, typename TModels, typename TSetter, TEMPLATE_REQUIRES(!TIsInvocable<TSetter>::Value)>
    void SyncViewModelCollection(TArray<TViewModel*, TAllocator>& ViewModels, const TModels& Models, TSetter&& Setter)
    {
        SyncViewModelCollection(ViewModels, Models, [] { return NewObject<TViewModel>(); }, Setter);
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
    void SyncViewModelCollection(TArray<TViewModel*, TAllocator>& ViewModels, const TModels& Models, TFactory&& Factory, TSetter&& Setter)
    {
        ViewModels.Reserve(Models.Num());

        while (ViewModels.Num() < Models.Num())
        {
            ViewModels.Add(Factory());
        }

        if (ViewModels.Num() > Models.Num())
        {
            ViewModels.RemoveAt(Models.Num(), ViewModels.Num() - Models.Num(), false);
        }

        int32 Index = 0;
        for (auto& Model : Models)
        {
            Setter(ViewModels[Index], Model);
            ++Index;
        }
    }
}