// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Misc/InlineValue.h"

class UBaseViewModel;
class FViewModelPropertyBase;

namespace UnrealMvvm_Impl
{
    struct IPropertyChangeHandler
    {
        virtual ~IPropertyChangeHandler() = default;
        virtual void Invoke(UBaseViewModel* ViewModel, const FViewModelPropertyBase* Property) const = 0;
    };

    struct FBindEntry
    {
        FBindEntry(const FViewModelPropertyBase* InProperty)
            : Property(InProperty)
        {
        }

        const FViewModelPropertyBase* Property;
        TInlineValue<IPropertyChangeHandler, sizeof(void*) * 4> Handler;
    };
}