// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Templates/Models.h"
#include "UObject/Class.h"

namespace UnrealMvvm_Impl
{

    /* Describes a type that can be compared using just operator== */
    struct CSimpleEqualityComparable
    {
        template <typename T>
        auto Requires(bool& Result, const T& A, const T& B) -> decltype(
            Result = A == B
        );
    };


    /* Helper class to determine whether we can compare values of type T */
    template <typename T>
    struct TCanCompareHelper
    {
        // check if type define comparison operator
        static constexpr bool Value = TModels<CSimpleEqualityComparable, T>::Value || TStructOpsTypeTraits<T>::WithIdentical;
    };

#define CAN_COMPARE_SPECIALIZE_CONTAINER(ContainerType) \
    template <typename TElementType> \
    struct TCanCompareHelper< ContainerType<TElementType> > \
    { \
        static constexpr bool Value = TCanCompareHelper<TElementType>::Value; \
    }

    /* Specialization for TArray */
    CAN_COMPARE_SPECIALIZE_CONTAINER(TArray);

    /* Specialization for TOptional */
    CAN_COMPARE_SPECIALIZE_CONTAINER(TOptional);

#undef CAN_COMPARE_SPECIALIZE_CONTAINER
}
