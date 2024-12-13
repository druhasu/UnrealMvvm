// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/Impl/Binding/BindingConfiguration.h"
#include "Containers/ArrayView.h"
#include "Containers/Array.h"

class FViewModelPropertyBase;

namespace UnrealMvvm_Impl
{
    struct FViewModelPropertyReflection;

    /*
     * Helper class to build BindingConfiguration from Native and Blueprint bindings
     */
    class UNREALMVVM_API FBindingConfigurationBuilder
    {
    public:
        FBindingConfigurationBuilder(UClass* ViewModelClass);

        void AddBinding(TArrayView<const FViewModelPropertyBase* const> PropertyPath);
        void AddBinding(TArrayView<const FName> PropertyPath);
        void AddBinding(TArrayView<const FViewModelPropertyReflection*> PropertyPath);

        FBindingConfiguration Build();

    private:
        struct FBindingTreeNode
        {
            UClass* ViewModelClass;
            const FViewModelPropertyBase* Property;
            bool bHandlerExpected = false;

            TArray<FBindingTreeNode> Children;

            friend bool operator== (const FBindingTreeNode& Node, const FViewModelPropertyBase* InProperty)
            {
                return Node.Property == InProperty;
            }
        };

        FBindingTreeNode Root;
    };

}
