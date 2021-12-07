// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Templates/ChooseClass.h"
#include "Templates/IsArithmetic.h"

namespace UnrealMvvm_Impl
{

    // helper used to select by-value setter for arithmetic types
    template <typename T>
    using TByValueOrByRef = TChooseClass< TIsArithmetic<T>::Value, T, const T& >;

    template <typename T>
    struct TPropertyTypeSelector
    {
        using GetterType = T;
        using SetterType = typename TByValueOrByRef<T>::Result;
        using FieldType = T;
    };

    template <typename T>
    struct TPropertyTypeSelector<const T&>
    {
        using GetterType = const T&;
        using SetterType = typename TByValueOrByRef<T>::Result;
        using FieldType = T;
    };

    template <typename T>
    struct TPropertyTypeSelector<T*>
    {
        using GetterType = T*;
        using SetterType = T*;
        using FieldType = T*;
    };

}