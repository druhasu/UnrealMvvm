// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "EdGraph/EdGraphNode.h"

class UBaseViewModel;
class FViewModelPropertyBase;

namespace UnrealMvvm_Impl
{

#if WITH_EDITOR
    // contains all supported pin values
    // mirrored from EdGraphSchema_K2 because it is not available in runtime
    enum class EPinCategoryType
    {
        Unsupported, // indicates that this property is not supported in Blueprint
        Boolean,
        Byte,
        Class, // SubCategoryObject is the MetaClass of the Class passed thru this pin, or SubCategory can be 'self'. The DefaultValue string should always be empty, use DefaultObject.
        SoftClass,
        Int,
        Int64,
        Float,
        Name,
        Object, // SubCategoryObject is the Class of the object passed thru this pin, or SubCategory can be 'self'. The DefaultValue string should always be empty, use DefaultObject.
        Interface, // SubCategoryObject is the Class of the object passed thru this pin.
        SoftObject, // SubCategoryObject is the Class of the AssetPtr passed thru this pin.
        String,
        Text,
        Struct, // SubCategoryObject is the ScriptStruct of the struct passed thru this pin, 'self' is not a valid SubCategory. DefaultObject should always be empty, the DefaultValue string may be used for supported structs.
        //Enum, // Looks like this value is not used by Unreal anymore. Byte is used instead with SubCategoryObject pointing to Enum
    };
#endif

    // contains reflection data about single viewmodel property
    struct UNREALMVVM_API FViewModelPropertyReflection
    {
        using FCopyValueFunction = TFunction< void (UBaseViewModel* /*ViewModel*/, void* /*OutValue*/, bool& /*OutHasValue*/) > ;

        const FViewModelPropertyBase* Property;
        FCopyValueFunction CopyValueToMemory;
        bool IsOptional = false;

#if WITH_EDITOR
        using SubCategoryGetterPtr = UObject* (*)();

        EPinCategoryType PinCategoryType;
        EPinCategoryType PinValueCategoryType; // for TMap properties
        EPinContainerType ContainerType;
        SubCategoryGetterPtr GetPinSubCategoryObject;
        SubCategoryGetterPtr GetPinValueSubCategoryObject; // for TMap properties
#endif
    };

}