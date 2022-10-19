// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Templates/UnrealTypeTraits.h"
#include "Templates/Decay.h"

namespace UnrealMvvm_Impl
{
    // checks what kind of type is T (Struct, Class or Interface)
    template <typename T>
    struct TValueTypeTraits
    {
        UE_STATIC_ASSERT_COMPLETE_TYPE(T, "T must be fully defined type, not just forward declared");

        template<typename U> static decltype(U::StaticStruct()) Test1(int);
        template<typename U> static decltype(U::StaticClass()) Test1(int);
        template<typename U> static char Test1(...);
        template<typename U> static decltype(&U::_getUObject) Test2(int);
        template<typename U> static char Test2(...);

        using JustT = typename TDecay<T>::Type;

        static const bool IsStruct = TIsSame<decltype(Test1<JustT>(0)), UScriptStruct*>::Value;
        static const bool IsClass = TIsSame<decltype(Test1<JustT>(0)), UClass*>::Value;
        static const bool IsInterface = !TIsSame< char, decltype(Test2<JustT>(0)) >::Value && !TIsDerivedFrom<JustT, UObject>::Value;
    };
}