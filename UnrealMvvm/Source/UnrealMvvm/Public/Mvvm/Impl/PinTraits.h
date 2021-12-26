// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/Impl/ViewModelPropertyReflection.h"
#include "Templates/IntegralConstant.h"

// forward declare all structs from Core module
struct FRotator;
struct FQuat;
struct FTransform;
struct FLinearColor;
struct FColor;
struct FPlane;
struct FVector;
struct FVector2D;
struct FVector4;
struct FRandomStream;
struct FGuid;
struct FBox2D;
struct FFallbackStruct;
struct FFloatRangeBound;
struct FFloatRange;
struct FInt32RangeBound;
struct FInt32Range;
struct FFloatInterval;
struct FInt32Interval;
struct FFrameNumber;
struct FFrameTime;
struct FSoftObjectPath;
struct FSoftClassPath;
struct FPrimaryAssetType;
struct FPrimaryAssetId;
struct FDateTime;
struct FPolyglotTextData;
struct FAssetBundleData;
struct FTestUninitializedScriptStructMembersTest;

namespace UnrealMvvm_Impl
{
#if WITH_EDITOR

    // checks what kind of type is T (Struct, Class or Interface)
    template <typename T>
    struct TPinTypeHelper
    {
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

    // Traits for structs from Core modules

    template <typename T>
    struct TIsBaseStructure : TIntegralConstant<bool, false> {};

#define DEFINE_BASE_STRUCTURE(ValueType) \
    template <> struct TIsBaseStructure<ValueType> : TIntegralConstant<bool, true> {}

    // List all structures from Class.h that have specializations for TBaseStructure
    DEFINE_BASE_STRUCTURE(FRotator);
    DEFINE_BASE_STRUCTURE(FQuat);
    DEFINE_BASE_STRUCTURE(FTransform);
    DEFINE_BASE_STRUCTURE(FLinearColor);
    DEFINE_BASE_STRUCTURE(FColor);
    DEFINE_BASE_STRUCTURE(FPlane);
    DEFINE_BASE_STRUCTURE(FVector);
    DEFINE_BASE_STRUCTURE(FVector2D);
    DEFINE_BASE_STRUCTURE(FVector4);
    DEFINE_BASE_STRUCTURE(FRandomStream);
    DEFINE_BASE_STRUCTURE(FGuid);
    DEFINE_BASE_STRUCTURE(FBox2D);
    DEFINE_BASE_STRUCTURE(FFallbackStruct);
    DEFINE_BASE_STRUCTURE(FInterpCurvePointFloat);
    DEFINE_BASE_STRUCTURE(FInterpCurvePointVector2D);
    DEFINE_BASE_STRUCTURE(FInterpCurvePointVector);
    DEFINE_BASE_STRUCTURE(FInterpCurvePointQuat);
    DEFINE_BASE_STRUCTURE(FInterpCurvePointTwoVectors);
    DEFINE_BASE_STRUCTURE(FInterpCurvePointLinearColor);
    DEFINE_BASE_STRUCTURE(FFloatRangeBound);
    DEFINE_BASE_STRUCTURE(FFloatRange);
    DEFINE_BASE_STRUCTURE(FInt32RangeBound);
    DEFINE_BASE_STRUCTURE(FInt32Range);
    DEFINE_BASE_STRUCTURE(FFloatInterval);
    DEFINE_BASE_STRUCTURE(FInt32Interval);
    DEFINE_BASE_STRUCTURE(FFrameNumber);
    DEFINE_BASE_STRUCTURE(FFrameTime);
    DEFINE_BASE_STRUCTURE(FSoftObjectPath);
    DEFINE_BASE_STRUCTURE(FSoftClassPath);
    DEFINE_BASE_STRUCTURE(FPrimaryAssetType);
    DEFINE_BASE_STRUCTURE(FPrimaryAssetId);
    DEFINE_BASE_STRUCTURE(FDateTime);
    DEFINE_BASE_STRUCTURE(FPolyglotTextData);
    DEFINE_BASE_STRUCTURE(FAssetBundleData);
    DEFINE_BASE_STRUCTURE(FTestUninitializedScriptStructMembersTest);

#undef DEFINE_BASE_STRUCTURE

    // Traits for Element type

    template <typename T, typename = void>
    struct TPinElementTraits
    {
        static const EPinCategoryType PinCategoryType = EPinCategoryType::Unsupported;
        static UObject* GetSubCategoryObject() { return nullptr; }
    };

#define DEFINE_SIMPLE_PIN_TRAITS(ValueType, InPinCategoryType) \
    template <> \
    struct TPinElementTraits< ValueType > \
    { \
        static const EPinCategoryType PinCategoryType = EPinCategoryType:: InPinCategoryType; \
        static UObject* GetSubCategoryObject() { return nullptr; } \
    }

#define DEFINE_COMPLEX_PIN_TRAITS(ValueType, InPinCategoryType, Condition, ObjectExpression) \
    template <typename T> \
    struct TPinElementTraits< ValueType, typename TEnableIf< Condition >::Type > \
    { \
        static const EPinCategoryType PinCategoryType = EPinCategoryType:: InPinCategoryType; \
        static UObject* GetSubCategoryObject() { return ObjectExpression; } \
    }

    // Boolean
    DEFINE_SIMPLE_PIN_TRAITS(bool, Boolean);

    // Byte
    DEFINE_SIMPLE_PIN_TRAITS(uint8, Byte);

    // Class. SubCategoryObject is the MetaClass of the Class passed thru this pin
    DEFINE_COMPLEX_PIN_TRAITS(TSubclassOf<T>, Class, TPinTypeHelper<T>::IsClass, T::StaticClass());

    // SoftClass
    DEFINE_COMPLEX_PIN_TRAITS(TSoftClassPtr<T>, SoftClass, TPinTypeHelper<T>::IsClass, T::StaticClass());

    // Int
    DEFINE_SIMPLE_PIN_TRAITS(int32, Int);

    // Int64
    DEFINE_SIMPLE_PIN_TRAITS(int64, Int64);

    // Float
    DEFINE_SIMPLE_PIN_TRAITS(float, Float);

    // Name
    DEFINE_SIMPLE_PIN_TRAITS(FName, Name);

    // Object. SubCategoryObject is the Class of the object passed thru this pin.
    DEFINE_COMPLEX_PIN_TRAITS(T*, Object, TPinTypeHelper<T>::IsClass, T::StaticClass());

    // Interface. SubCategoryObject is the Class of the object passed thru this pin.
    DEFINE_COMPLEX_PIN_TRAITS(TScriptInterface<T>, Interface, TPinTypeHelper<T>::IsInterface, T::UClassType::StaticClass());

    // SoftObject. SubCategoryObject is the Class of the AssetPtr passed thru this pin.
    DEFINE_COMPLEX_PIN_TRAITS(TSoftObjectPtr<T>, SoftObject, TPinTypeHelper<T>::IsClass, T::StaticClass());

    // String
    DEFINE_SIMPLE_PIN_TRAITS(FString, String);

    // Text
    DEFINE_SIMPLE_PIN_TRAITS(FText, Text);

    // Struct. SubCategoryObject is the ScriptStruct of the struct passed thru this pin
    DEFINE_COMPLEX_PIN_TRAITS(T, Struct, TPinTypeHelper<T>::IsStruct, TDecay<T>::Type::StaticStruct());

    // Struct from Core
    DEFINE_COMPLEX_PIN_TRAITS(T, Struct, TIsBaseStructure<typename TDecay<T>::Type>::Value, TBaseStructure<typename TDecay<T>::Type>::Get());

    // Enum. SubCategoryObject is the UEnum object passed thru this pin.
    DEFINE_COMPLEX_PIN_TRAITS(T, Byte, TIsEnumClass<T>::Value, StaticEnum<T>());

#undef DEFINE_SIMPLE_PIN_TRAITS
#undef DEFINE_COMPLEX_PIN_TRAITS

    // Traits for single element container

    struct FPinNoValueTraits
    {
        static const EPinCategoryType PinValueCategoryType = EPinCategoryType::Unsupported;
        static UObject* GetValueSubCategoryObject() { return nullptr; }
    };

    // Traits for whole Pin

    template <typename T>
    struct TPinTraits : public TPinElementTraits<T>, public FPinNoValueTraits
    {
        static const EPinContainerType PinContainerType = EPinContainerType::None;
    };

    template <typename T>
    struct TPinTraits< TArray<T> > : public TPinElementTraits<T>, public FPinNoValueTraits
    {
        static const EPinContainerType PinContainerType = EPinContainerType::Array;
    };

    template <typename T>
    struct TPinTraits< TSet<T> > : public TPinElementTraits<T>, public FPinNoValueTraits
    {
        static const EPinContainerType PinContainerType = EPinContainerType::Set;
    };

    template <typename TKey, typename TValue>
    struct TPinTraits< TMap<TKey, TValue> > : public TPinElementTraits<TKey>
    {
        static const EPinContainerType PinContainerType = EPinContainerType::Map;

        static const EPinCategoryType PinValueCategoryType = TPinElementTraits<TValue>::PinCategoryType;
        static UObject* GetValueSubCategoryObject() { return TPinElementTraits<TValue>::GetSubCategoryObject(); }
    };

#endif
}