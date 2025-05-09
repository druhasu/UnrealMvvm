// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/Impl/Property/ViewModelPropertyReflection.h"
#include "Mvvm/Impl/Utils/TryGetStaticEnum.h"
#include "Templates/IntegralConstant.h"
#include "Mvvm/Impl/Property/ValueTypeTraits.h"
#include "Math/MathFwd.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Misc/EngineVersionComparison.h"

// forward declare all structs from Core module
struct FLinearColor;
struct FColor;
struct FRandomStream;
struct FGuid;
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
struct FFrameRate;

// special case for FTimespan, because it does not have builtin TBaseStruct for some reason
template<>
struct TBaseStructure<FTimespan>
{
    static UNREALMVVM_API UScriptStruct* Get()
    {
        // copied from StaticGetBaseStructureInternal
        static UPackage* CoreUObjectPkg = FindObjectChecked<UPackage>(nullptr, TEXT("/Script/CoreUObject"));
        static UScriptStruct* Result = (UScriptStruct*)StaticFindObjectFastInternal(UScriptStruct::StaticClass(), CoreUObjectPkg, "Timespan", false, RF_NoFlags, EInternalObjectFlags::None);
        return Result;
    }
};

namespace UnrealMvvm_Impl
{
#if WITH_EDITOR
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
    DEFINE_BASE_STRUCTURE(FTimespan);

#if !UE_VERSION_OLDER_THAN(5,1,0)
    // these types were exposed in 5.1
    DEFINE_BASE_STRUCTURE(FIntPoint);
    DEFINE_BASE_STRUCTURE(FIntVector);
    DEFINE_BASE_STRUCTURE(FIntVector4);
    DEFINE_BASE_STRUCTURE(FDoubleRangeBound);
    DEFINE_BASE_STRUCTURE(FDoubleRange);
    DEFINE_BASE_STRUCTURE(FDoubleInterval);
    DEFINE_BASE_STRUCTURE(FTopLevelAssetPath);
#endif

#if !UE_VERSION_OLDER_THAN(5,2,0)
    // these types were exposed in 5.2
    DEFINE_BASE_STRUCTURE(FRay);
    DEFINE_BASE_STRUCTURE(FSphere);
#endif

#if !UE_VERSION_OLDER_THAN(5,5,0)
    // these types were exposed in 5.5
    DEFINE_BASE_STRUCTURE(FInt64Vector2);
    DEFINE_BASE_STRUCTURE(FFrameRate);
#endif

#undef DEFINE_BASE_STRUCTURE

    // Traits for Pin Category

    template <typename T, typename = void>
    struct TPinCategoryTraits
    {
        static const EPinCategoryType PinCategoryType = EPinCategoryType::Unsupported;
        static UObject* GetSubCategoryObject() { return nullptr; }
    };

#define DEFINE_SIMPLE_PIN_TRAITS(ValueType, InPinCategoryType) \
    template <> \
    struct TPinCategoryTraits< ValueType > \
    { \
        static const EPinCategoryType PinCategoryType = EPinCategoryType:: InPinCategoryType; \
        static UObject* GetSubCategoryObject() { return nullptr; } \
    }

#define DEFINE_COMPLEX_PIN_TRAITS(ValueType, InPinCategoryType, Condition, ObjectExpression) \
    template <typename T> \
    struct TPinCategoryTraits< ValueType, typename TEnableIf< Condition >::Type > \
    { \
        static const EPinCategoryType PinCategoryType = EPinCategoryType:: InPinCategoryType; \
        static UObject* GetSubCategoryObject() { return ObjectExpression; } \
    }

    // Boolean
    DEFINE_SIMPLE_PIN_TRAITS(bool, Boolean);

    // Byte
    DEFINE_SIMPLE_PIN_TRAITS(uint8, Byte);

    // Class. SubCategoryObject is the MetaClass of the Class passed thru this pin
    DEFINE_COMPLEX_PIN_TRAITS(TSubclassOf<T>, Class, TValueTypeTraits<T>::IsClass, T::StaticClass());

    // SoftClass
    DEFINE_COMPLEX_PIN_TRAITS(TSoftClassPtr<T>, SoftClass, TValueTypeTraits<T>::IsClass, T::StaticClass());

    // Int
    DEFINE_SIMPLE_PIN_TRAITS(int32, Int);

    // Int64
    DEFINE_SIMPLE_PIN_TRAITS(int64, Int64);

    // Float
    DEFINE_SIMPLE_PIN_TRAITS(float, Float);

#if ENGINE_MAJOR_VERSION >= 5
    // Double
    DEFINE_SIMPLE_PIN_TRAITS(double, Double);
#endif

    // Name
    DEFINE_SIMPLE_PIN_TRAITS(FName, Name);

    // Object. SubCategoryObject is the Class of the object passed thru this pin.
    DEFINE_COMPLEX_PIN_TRAITS(T*, Object, TValueTypeTraits<T>::IsClass, T::StaticClass());

#if ENGINE_MAJOR_VERSION >= 5
    // Object inside TObjectPtr
    DEFINE_COMPLEX_PIN_TRAITS(TObjectPtr<T>, Object, TValueTypeTraits<T>::IsClass, T::StaticClass());
#endif

    // Interface. SubCategoryObject is the Class of the object passed thru this pin.
    DEFINE_COMPLEX_PIN_TRAITS(TScriptInterface<T>, Interface, TValueTypeTraits<T>::IsInterface, T::UClassType::StaticClass());

    // SoftObject. SubCategoryObject is the Class of the AssetPtr passed thru this pin.
    DEFINE_COMPLEX_PIN_TRAITS(TSoftObjectPtr<T>, SoftObject, TValueTypeTraits<T>::IsClass, T::StaticClass());

    // String
    DEFINE_SIMPLE_PIN_TRAITS(FString, String);

    // Text
    DEFINE_SIMPLE_PIN_TRAITS(FText, Text);

    // Struct. SubCategoryObject is the ScriptStruct of the struct passed thru this pin
    DEFINE_COMPLEX_PIN_TRAITS(T, Struct, TValueTypeTraits<T>::IsStruct, TDecay<T>::Type::StaticStruct());

    // Struct from Core
    DEFINE_COMPLEX_PIN_TRAITS(T, Struct, TIsBaseStructure<typename TDecay<T>::Type>::Value, TBaseStructure<typename TDecay<T>::Type>::Get());

    // Enum. SubCategoryObject is the UEnum object passed thru this pin.
    DEFINE_COMPLEX_PIN_TRAITS(T, Enum, TIsEnumClass<T>::Value, TryGetStaticEnum<T>());

#undef DEFINE_SIMPLE_PIN_TRAITS
#undef DEFINE_COMPLEX_PIN_TRAITS

    // Traits for container without separate Value

    struct FPinContainerNoValueTraits
    {
        static const EPinCategoryType PinValueCategoryType = EPinCategoryType::Unsupported;
        static UObject* GetValueSubCategoryObject() { return nullptr; }
    };

    // Traits for container of a Pin

    template <typename T>
    struct TPinContainerTraits : public TPinCategoryTraits<T>, public FPinContainerNoValueTraits
    {
        static const EPinContainerType PinContainerType = EPinContainerType::None;
    };

    template <typename T>
    struct TPinContainerTraits< TArray<T> > : public TPinCategoryTraits<T>, public FPinContainerNoValueTraits
    {
        static const EPinContainerType PinContainerType = EPinContainerType::Array;
    };

    template <typename T>
    struct TPinContainerTraits< TSet<T> > : public TPinCategoryTraits<T>, public FPinContainerNoValueTraits
    {
        static const EPinContainerType PinContainerType = EPinContainerType::Set;
    };

    template <typename TKey, typename TValue>
    struct TPinContainerTraits< TMap<TKey, TValue> > : public TPinCategoryTraits<TKey>
    {
        static const EPinContainerType PinContainerType = EPinContainerType::Map;

        static const EPinCategoryType PinValueCategoryType = TPinCategoryTraits<TValue>::PinCategoryType;
        static UObject* GetValueSubCategoryObject() { return TPinCategoryTraits<TValue>::GetSubCategoryObject(); }
    };

#else

    // Non-Editor base class. Empty to reduce build times
    template <typename T>
    struct TPinContainerTraits {};

#endif

    // Traits for whole Pin

    template <typename T>
    struct TPinTraits : public TPinContainerTraits<T>
    {
        static const bool IsOptional = false;
    };

    template <typename T>
    struct TPinTraits< TOptional<T> > : public TPinContainerTraits<T>
    {
        static const bool IsOptional = true;
    };

}
