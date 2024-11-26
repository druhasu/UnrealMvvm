// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Templates/ChooseClass.h"
#include "Templates/IsArithmetic.h"
#include "Mvvm/ViewModelPropertyTypeTraits.h"
#include "Misc/EngineVersionComparison.h"

#if !UE_VERSION_OLDER_THAN(5,4,0)
#include <type_traits>
#endif

namespace UnrealMvvm_Impl
{

    // helper used to select by-value setter for arithmetic types
//#if UE_VERSION_OLDER_THAN(5,4,0)
    //template <typename T>
    //using TByValueOrByRef = TChooseClass< TIsArithmetic<T>::Value || TIsEnum<T>::Value || TViewModelPropertyTypeTraits<T>::WithSetterArgumentByValue, T, const T& >::Result;
//#else
    template <typename T>
    using TByValueOrByRef = std::conditional_t< TIsArithmetic<T>::Value || TIsEnum<T>::Value || TViewModelPropertyTypeTraits<T>::WithSetterArgumentByValue, T, const T& >;
//#endif

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
