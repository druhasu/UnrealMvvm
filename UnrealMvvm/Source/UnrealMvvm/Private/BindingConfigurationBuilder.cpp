// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Mvvm/Impl/Binding/BindingConfigurationBuilder.h"
#include "Mvvm/Impl/Binding/BindingConfiguration.h"
#include "Mvvm/Impl/Property/ViewModelRegistry.h"

namespace UnrealMvvm_Impl
{

FBindingConfigurationBuilder::FBindingConfigurationBuilder(UClass* ViewModelClass)
{
    Root.ViewModelClass = ViewModelClass;
}

void FBindingConfigurationBuilder::AddBinding(TArrayView<const FViewModelPropertyBase* const> PropertyPath)
{
    check(PropertyPath.Num() > 0);

    TArray<const FViewModelPropertyReflection*, TInlineAllocator<8>> ResolvedPath;

    UClass* CurrentViewModelClass = Root.ViewModelClass;
    for (int32 PropertyIndex = 0; PropertyIndex < PropertyPath.Num(); ++PropertyIndex)
    {
        check(CurrentViewModelClass);

        const FViewModelPropertyReflection* Reflection = FViewModelRegistry::FindProperty(CurrentViewModelClass, PropertyPath[PropertyIndex]->GetName());
        if (Reflection == nullptr)
        {
            // ignore whole binding if it has invalid entry
            return;
        }

        ResolvedPath.Emplace(Reflection);
        CurrentViewModelClass = Reflection->GetOperations().GetValueClass();
    }

    AddBinding(ResolvedPath);
}

void FBindingConfigurationBuilder::AddBinding(TArrayView<const FName> PropertyPath)
{
    check(PropertyPath.Num() > 0);

    TArray<const FViewModelPropertyReflection*, TInlineAllocator<8>> ResolvedPath;

    UClass* CurrentViewModelClass = Root.ViewModelClass;
    for (int32 PropertyIndex = 0; PropertyIndex < PropertyPath.Num(); ++PropertyIndex)
    {
        check(CurrentViewModelClass);

        const FViewModelPropertyReflection* Reflection = FViewModelRegistry::FindProperty(CurrentViewModelClass, PropertyPath[PropertyIndex]);
        if (Reflection == nullptr)
        {
            // ignore whole binding if it has invalid entry
            return;
        }

        ResolvedPath.Emplace(Reflection);
        CurrentViewModelClass = Reflection->GetOperations().GetValueClass();
    }

    AddBinding(ResolvedPath);
}

void FBindingConfigurationBuilder::AddBinding(TArrayView<const FViewModelPropertyReflection*> PropertyPath)
{
    check(PropertyPath.Num() > 0);

    FBindingTreeNode* CurrentNode = &Root;
    for (int32 PropertyIndex = 0; PropertyIndex < PropertyPath.Num(); ++PropertyIndex)
    {
        const FViewModelPropertyReflection* Reflection = PropertyPath[PropertyIndex];
        check(Reflection);

        const FViewModelPropertyBase* Property = Reflection->GetProperty();
        FBindingTreeNode* NewNode = CurrentNode->Children.FindByKey(Property);
        if (NewNode == nullptr || PropertyIndex == PropertyPath.Num() - 1)
        {
            NewNode = &CurrentNode->Children.Add_GetRef(FBindingTreeNode{ Reflection->GetOperations().GetValueClass(), Property });
        }

        CurrentNode = NewNode;
    }

    check(!CurrentNode->bHandlerExpected);
    CurrentNode->bHandlerExpected = true;
}

FBindingConfiguration FBindingConfigurationBuilder::Build()
{
    // queue for breadth-first tree iteration
    TResizableCircularQueue<TTuple<FBindingTreeNode*, int32>> Queue;

    // count how many entries of each type do we need
    int32 NumViewModels = 0;
    int32 NumProperties = 0;

    Queue.Enqueue(MakeTuple(&Root, INDEX_NONE));
    while (!Queue.IsEmpty())
    {
        TTuple<FBindingTreeNode*, int32> CurrentNode = Queue.PeekNoCheck();
        Queue.PopNoCheck();

        if (CurrentNode.Key->Children.Num() > 0)
        {
            NumViewModels += 1;

            for (FBindingTreeNode& Node : CurrentNode.Key->Children)
            {
                Queue.Enqueue(MakeTuple(&Node, INDEX_NONE));
                NumProperties += 1;
            }
        }
    }

    if (NumProperties == 0)
    {
        // if no properties are bound, return empty configuration
        return FBindingConfiguration();
    }

    // allocate memory for all entries and initialize it
    FBindingConfiguration Result(NumViewModels, NumProperties);

    // pack bindings tree into linear lookup data structure
    TArrayView<FResolvedViewModelEntry> ViewModels = Result.GetViewModels();
    TArrayView<FResolvedPropertyEntry> Properties = Result.GetProperties();

    int32 NextViewModelIndex = 0;
    int32 NextPropertyIndex = 0;

    Queue.Enqueue(MakeTuple(&Root, INDEX_NONE));
    while (!Queue.IsEmpty())
    {
        TTuple<FBindingTreeNode*, int32> CurrentNode = Queue.PeekNoCheck();
        Queue.PopNoCheck();

        if (CurrentNode.Value != INDEX_NONE)
        {
            Properties[CurrentNode.Value].NextViewModelIndex = NextViewModelIndex;
        }

        ViewModels[NextViewModelIndex] = FResolvedViewModelEntry{ { CurrentNode.Key->ViewModelClass }, NextPropertyIndex, CurrentNode.Key->Children.Num() };
        NextViewModelIndex++;

        for (FBindingTreeNode& Node : CurrentNode.Key->Children)
        {
            if (Node.Children.Num() > 0)
            {
                Queue.Enqueue(MakeTuple(&Node, NextPropertyIndex));
            }

            Properties[NextPropertyIndex] = FResolvedPropertyEntry(Node.Property, INDEX_NONE);
            NextPropertyIndex++;
        }
    }

    return Result;
}

}
