// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Runtime/Launch/Resources/Version.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"
#include "Templates/IsEnumClass.h"

namespace UnrealMvvm_Impl
{
    // StaticEnum<T> is defined only if T is UEnum. That prevents linking if called with incorrect T
    // We cannot determine in compile time whether T is UEnum or not. So we had to do it in runtime
    // This function is a replacement for StaticEnum<T> and returns nullptr if corresponding UEnum class is not found for T

    template <typename T>
    UObject* TryFindEnum()
    {
#if defined(__clang__)

        // clang format: UObject *__cdecl UnrealMvvm_Impl::TryFindEnum()[T = EMyEnum]
        const TCHAR StartMarker[]{ TEXT("[T = ") };
        const TCHAR EndMarker[]{ TEXT("]") };
        FString PrettyName(__PRETTY_FUNCTION__);

#elif defined(__GNUC__) && !defined(__clang__)

        // GCC format: UObject *__cdecl UnrealMvvm_Impl::TryFindEnum()[with T = EMyEnum]
        const TCHAR StartMarker[]{ TEXT("[with T = ") };
        const TCHAR EndMarker[]{ TEXT("]") };
        FString PrettyName(__PRETTY_FUNCTION__);

#elif defined(_MSC_VER)

        // MSVC format: UObject *__cdecl UnrealMvvm_Impl::TryFindEnum<enum EMyEnum>(void)
        const TCHAR StartMarker[]{ TEXT("<enum ") };
        const TCHAR EndMarker[]{ TEXT(">") };
        FString PrettyName(__FUNCSIG__);

#else
        #error "No support for this compiler."
#endif
        const int32 StartMarkerLength = (sizeof(StartMarker) / sizeof(StartMarker[0])) - 1; // StartMarker contains trailing zero, so -1 is required

        int32 StartIndex = PrettyName.Find(StartMarker) + StartMarkerLength;
        int32 EndIndex = PrettyName.Find(EndMarker, ESearchCase::IgnoreCase, ESearchDir::FromStart, StartIndex);

        FString EnumName = PrettyName.Mid(StartIndex, EndIndex - StartIndex);

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1
        return FindFirstObject<UEnum>(*EnumName);
#else
        return FindObject<UEnum>(ANY_PACKAGE, *EnumName);
#endif
    }

    template <typename T>
    UObject* TryGetStaticEnum()
    {
        static_assert(TIsEnumClass<T>::Value, "T must be a enum class");

        static UObject* Result = TryFindEnum<T>();
        return Result;
    }

}
