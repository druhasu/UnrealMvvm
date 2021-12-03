// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

namespace UnrealMvvm_Impl
{

    template <typename T>
    struct TPropertyTypeSelector
    {
        using GetterType = T;
        using SetterType = const T&;
        using FieldType = T;
    };

    template <typename T>
    struct TPropertyTypeSelector<const T&>
    {
        using GetterType = const T&;
        using SetterType = const T&;
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