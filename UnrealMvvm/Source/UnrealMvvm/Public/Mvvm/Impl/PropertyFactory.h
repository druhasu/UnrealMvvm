// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "UObject/GarbageCollection.h"
#include "Mvvm/Impl/ValueTypeTraits.h"

namespace UnrealMvvm_Impl
{

    namespace Details
    {
        /*
         * This class creates FProperty objects based on requested TValue
         */
        template <typename TValue, typename = void>
        struct TPropertyFactory
        {
            static constexpr bool IsSupportedByUnreal = false;
            static constexpr bool ContainsObjectReferences = false;

            static void AddProperty(FFieldVariant Scope, int32 FieldOffset, const FName& DebugName)
            {
            }
        };

#define DECLARE_SIMPLE_PROPERTY(VariableType, PropertyType, ...) \
        template <> \
        struct TPropertyFactory<VariableType> \
        { \
            static constexpr bool IsSupportedByUnreal = true; \
            static constexpr bool ContainsObjectReferences = false; \
            static void AddProperty(FFieldVariant Scope, int32 FieldOffset, const FName& DebugName) \
            { \
                new PropertyType(Scope, DebugName, EObjectFlags::RF_NoFlags, FieldOffset, EPropertyFlags::CPF_None, ##__VA_ARGS__); \
            } \
        }

#define DECLARE_WRAPPER_PROPERTY(VariableType, PropertyType, ContainsReferences, InnerClass) \
        template <typename TValue> \
        struct TPropertyFactory<VariableType> \
        { \
            static constexpr bool IsSupportedByUnreal = true; \
            static constexpr bool ContainsObjectReferences = ContainsReferences; \
            static void AddProperty(FFieldVariant Scope, int32 FieldOffset, const FName& DebugName) \
            { \
                new PropertyType(Scope, DebugName, EObjectFlags::RF_NoFlags, FieldOffset, EPropertyFlags::CPF_None, InnerClass); \
            } \
        }

        DECLARE_SIMPLE_PROPERTY(bool, FBoolProperty, 1, sizeof(bool), true);
        DECLARE_SIMPLE_PROPERTY(uint8, FByteProperty);
        DECLARE_SIMPLE_PROPERTY(double, FDoubleProperty);
        DECLARE_SIMPLE_PROPERTY(float, FFloatProperty);
        DECLARE_SIMPLE_PROPERTY(int16, FInt16Property);
        DECLARE_SIMPLE_PROPERTY(int64, FInt64Property);
        DECLARE_SIMPLE_PROPERTY(int8, FInt8Property);
        DECLARE_SIMPLE_PROPERTY(int32, FIntProperty);
        DECLARE_SIMPLE_PROPERTY(FName, FNameProperty);
        DECLARE_SIMPLE_PROPERTY(FString, FStrProperty);
        DECLARE_SIMPLE_PROPERTY(uint16, FUInt16Property);
        DECLARE_SIMPLE_PROPERTY(uint32, FUInt32Property);
        DECLARE_SIMPLE_PROPERTY(uint64, FUInt64Property);

        DECLARE_WRAPPER_PROPERTY(TScriptInterface<TValue>, FInterfaceProperty, true, TValue::UClassType::StaticClass());
        DECLARE_WRAPPER_PROPERTY(TLazyObjectPtr<TValue>, FLazyObjectProperty, false, TValue::StaticClass());
        DECLARE_WRAPPER_PROPERTY(TSoftClassPtr<TValue>, FSoftClassProperty, false, TValue::StaticClass());
        DECLARE_WRAPPER_PROPERTY(TSoftObjectPtr<TValue>, FSoftObjectProperty, false, TValue::StaticClass());
        DECLARE_WRAPPER_PROPERTY(TWeakObjectPtr<TValue>, FWeakObjectProperty, false, TValue::StaticClass());

#undef DECLARE_SIMPLE_PROPERTY
#undef DECLARE_WRAPPER_PROPERTY

        /* UObject pointer */
        template <typename TValue>
        struct TPropertyFactory<TValue*, typename TEnableIf<TValueTypeTraits<TValue>::IsClass>::Type>
        {
            static constexpr bool IsSupportedByUnreal = true;
            static constexpr bool ContainsObjectReferences = true;

            static void AddProperty(FFieldVariant Scope, int32 FieldOffset, const FName& DebugName)
            {
                new FObjectProperty(Scope, DebugName, EObjectFlags::RF_NoFlags, FieldOffset, EPropertyFlags::CPF_None, TValue::StaticClass());
            }
        };

        /* UStruct value */
        template <typename TValue>
        struct TPropertyFactory<TValue, typename TEnableIf<TValueTypeTraits<TValue>::IsStruct>::Type>
        {
            static constexpr bool IsSupportedByUnreal = true;
            static constexpr bool ContainsObjectReferences = true; // assume true, we cannot easily check this in compile time

            static void AddProperty(FFieldVariant Scope, int32 FieldOffset, const FName& DebugName)
            {
                new FStructProperty(Scope, DebugName, EObjectFlags::RF_NoFlags, FieldOffset, EPropertyFlags::CPF_None, TValue::StaticStruct());
            }
        };

        /* TArray<> */
        template <typename TValue>
        struct TPropertyFactory<TArray<TValue>>
        {
            static constexpr bool IsSupportedByUnreal = true;
            static constexpr bool ContainsObjectReferences = TPropertyFactory<TValue>::ContainsObjectReferences;

            static void AddProperty(FFieldVariant Scope, int32 FieldOffset, const FName& DebugName)
            {
                if (ContainsObjectReferences)
                {
                    auto Prop = new FArrayProperty(Scope, DebugName, EObjectFlags::RF_NoFlags, FieldOffset, EPropertyFlags::CPF_None, EArrayPropertyFlags::None);
                    TPropertyFactory<TValue>::AddProperty(Prop, FieldOffset, FName(DebugName.ToString() + TEXT("_Value")));
                }
            }
        };

        /* TSet<> */
        template <typename TValue>
        struct TPropertyFactory<TSet<TValue>>
        {
            static constexpr bool IsSupportedByUnreal = true;
            static constexpr bool ContainsObjectReferences = TPropertyFactory<TValue>::ContainsObjectReferences;

            static void AddProperty(FFieldVariant Scope, int32 FieldOffset, const FName& DebugName)
            {
                if (ContainsObjectReferences)
                {
                    auto Prop = new FSetProperty(Scope, DebugName, EObjectFlags::RF_NoFlags, FieldOffset, EPropertyFlags::CPF_None);
                    TPropertyFactory<TValue>::AddProperty(Prop, FieldOffset, FName(DebugName.ToString() + TEXT("_Value")));
                }
            }
        };

        /* TMap<,> */
        template <typename TKey, typename TValue>
        struct TPropertyFactory<TMap<TKey, TValue>>
        {
            static constexpr bool IsSupportedByUnreal = true;
            static constexpr bool ContainsObjectReferences = TPropertyFactory<TKey>::ContainsObjectReferences || TPropertyFactory<TValue>::ContainsObjectReferences;

            static void AddProperty(FFieldVariant Scope, int32 FieldOffset, const FName& DebugName)
            {
                // if either TKey or TValue has references, then both of them MUST be supported
                static_assert(!ContainsObjectReferences || TPropertyFactory<TKey>::IsSupportedByUnreal, "Unsupported Key Type");
                static_assert(!ContainsObjectReferences || TPropertyFactory<TValue>::IsSupportedByUnreal, "Unsupported Value Type");

                if (ContainsObjectReferences)
                {
                    auto Prop = new FMapProperty(Scope, DebugName, EObjectFlags::RF_NoFlags, FieldOffset, EPropertyFlags::CPF_None, EMapPropertyFlags::None);
                    TPropertyFactory<TKey>::AddProperty(Prop, 0, FName(DebugName.ToString() + TEXT("_Key")));
                    TPropertyFactory<TValue>::AddProperty(Prop, 1, FName(DebugName.ToString() + TEXT("_Value")));
                }
            }
        };

    }

}