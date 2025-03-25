// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Templates/IsArithmetic.h"
#include "Templates/IsEnum.h"
#include "Mvvm/ViewModelPropertyTypeTraits.h"
#include <type_traits>

namespace UnrealMvvm_Impl
{

    // helper used to select by-value setter for arithmetic types
    template <typename T>
    using TByValueOrByRef = std::conditional_t< TIsArithmetic<T>::Value || TIsEnum<T>::Value || TViewModelPropertyTypeTraits<T>::WithSetterArgumentByValue, T, const T& >;

    template <typename T>
    struct TPropertyTypeSelector
    {
        using GetterType = T;
        using SetterType = TByValueOrByRef<T>;
        using FieldType = T;
    };

    template <typename T>
    struct TPropertyTypeSelector<const T&>
    {
        using GetterType = const T&;
        using SetterType = TByValueOrByRef<T>;
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
