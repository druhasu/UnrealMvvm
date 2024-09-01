// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Mvvm/Impl/Binding/BindingConfigurationBuilder.h"
#include "Mvvm/Impl/Binding/BindingConfiguration.h"

namespace UnrealMvvm_Impl
{

FBindingConfigurationBuilder::FBindingConfigurationBuilder(UClass* ViewModelClass)
{
    Root.ViewModelClass = ViewModelClass;
}

void FBindingConfigurationBuilder::AddBinding(TArrayView<const FViewModelPropertyBase* const> PropertyPath)
{
    check(PropertyPath.Num() > 0);

    UClass* CurrentViewModelClass = Root.ViewModelClass;
    FBindingTreeNode* CurrentNode = &Root;

    for (int32 PropertyIndex = 0; PropertyIndex < PropertyPath.Num(); ++PropertyIndex)
    {
        const FViewModelPropertyBase* Property = PropertyPath[PropertyIndex];
        check(CurrentViewModelClass);

        const FViewModelPropertyReflection* Reflection = FViewModelRegistry::FindProperty(CurrentViewModelClass, Property->GetName());
        CurrentViewModelClass = Reflection->GetOperations().GetValueClass();

        FBindingTreeNode* NewNode = CurrentNode->Children.FindByKey(Property);
        if (NewNode == nullptr || PropertyIndex == PropertyPath.Num() - 1)
        {
            NewNode = &CurrentNode->Children.Add_GetRef(FBindingTreeNode{ CurrentViewModelClass, Property });
        }

        CurrentNode = NewNode;
    }

    check(!CurrentNode->bHandlerExpected);
    CurrentNode->bHandlerExpected = true;
}

void FBindingConfigurationBuilder::AddBinding(TArrayView<const FName> PropertyPath)
{
    check(PropertyPath.Num() > 0);

    UClass* CurrentViewModelClass = Root.ViewModelClass;
    FBindingTreeNode* CurrentNode = &Root;

    for (int32 PropertyIndex = 0; PropertyIndex < PropertyPath.Num(); ++PropertyIndex)
    {
        FName PropertyName = PropertyPath[PropertyIndex];
        check(CurrentViewModelClass);

        const FViewModelPropertyReflection* Reflection = FViewModelRegistry::FindProperty(CurrentViewModelClass, PropertyName);
        const FViewModelPropertyBase* Property = Reflection->GetProperty();
        CurrentViewModelClass = Reflection->GetOperations().GetValueClass();

        FBindingTreeNode* NewNode = CurrentNode->Children.FindByKey(Property);
        if (NewNode == nullptr || PropertyIndex == PropertyPath.Num() - 1)
        {
            NewNode = &CurrentNode->Children.Add_GetRef(FBindingTreeNode{ CurrentViewModelClass, Property });
        }

        CurrentNode = NewNode;
    }

    check(!CurrentNode->bHandlerExpected);
    CurrentNode->bHandlerExpected = true;
}

FBindingConfiguration FBindingConfigurationBuilder::Build()
{
    // queue for breath-first tree iteration
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

        ViewModels[NextViewModelIndex] = FResolvedViewModelEntry{ CurrentNode.Key->ViewModelClass, NextPropertyIndex, CurrentNode.Key->Children.Num() };
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
