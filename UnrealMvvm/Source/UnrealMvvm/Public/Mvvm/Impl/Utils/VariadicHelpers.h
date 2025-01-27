// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include <type_traits>
#include "Templates/Identity.h"

namespace UnrealMvvm_Impl
{

    /* Returns First type from argument pack */
    template<typename... TArgs>
    struct TFirstType;

    /* Returns First type from argument pack */
    template<typename T, typename... TArgs>
    struct TFirstType<T, TArgs...> { using Type = T; };

    /* Returns First type from argument pack */
    template<typename... TArgs>
    using TFirstType_T = typename TFirstType<TArgs...>::Type;


    /* Returns Last type from argument pack */
    template<typename... TArgs>
    struct TLastType
    {
        // Use a fold-expression to fold the comma operator over the parameter pack.
        using Type = typename decltype((TIdentity<TArgs>{}, ...))::Type;
    };

    /* Returns Last type from argument pack */
    template<typename... TArgs>
    using TLastType_T = typename TLastType<TArgs...>::Type;

    /* Removes const and pointer from a type */
    template <typename T>
    using TJustType_T = std::remove_const_t<std::remove_pointer_t<T>>;

}
