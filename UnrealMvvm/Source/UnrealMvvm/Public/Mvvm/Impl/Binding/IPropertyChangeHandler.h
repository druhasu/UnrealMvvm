// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

class UBaseViewModel;
class FViewModelPropertyBase;

namespace UnrealMvvm_Impl
{
    struct IPropertyChangeHandler
    {
        virtual ~IPropertyChangeHandler() = default;
        virtual void Invoke(UBaseViewModel* ViewModel, const FViewModelPropertyBase* Property) const = 0;
    };
}
