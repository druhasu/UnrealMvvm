// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "EdGraph/EdGraphNode.h"
#include "Templates/TypeCompatibleBytes.h"

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
        Double,
        Name,
        Object, // SubCategoryObject is the Class of the object passed thru this pin, or SubCategory can be 'self'. The DefaultValue string should always be empty, use DefaultObject.
        Interface, // SubCategoryObject is the Class of the object passed thru this pin.
        SoftObject, // SubCategoryObject is the Class of the AssetPtr passed thru this pin.
        String,
        Text,
        Struct, // SubCategoryObject is the ScriptStruct of the struct passed thru this pin, 'self' is not a valid SubCategory. DefaultObject should always be empty, the DefaultValue string may be used for supported structs.
        Enum, // Looks like this value is not used by Unreal anymore. Byte is used instead with SubCategoryObject pointing to Enum. But we are using this to mark properties that should be enums
    };
#endif

    // contains operations that can be performed with property
    struct UNREALMVVM_API FViewModelPropertyOperations
    {
        virtual ~FViewModelPropertyOperations() {}

        // Reads value from InViewModel and writes to memory pointed by OutValue. OutHasValue denotes whether TOptional property has value
        virtual void GetValue(UBaseViewModel* InViewModel, void* OutValue, bool& OutHasValue) const = 0;

        // Writes value to InViewModel from memory pointer by InValue
        virtual void SetValue(UBaseViewModel* InViewModel, void* InValue, bool InHasValue) const = 0;

        // Adds new FProperty to given class
        virtual void AddClassProperty(UClass* TargetClass) const = 0;

        // Returns whether this property might contain Object Reference for GC
        virtual bool ContainsObjectReference(bool bIncludeNoFieldProperties) const = 0;

        // Returns UClass of owning ViewModel
        virtual UClass* GetViewModelClass() const = 0;

        // Returns UClass of value contained inside the property. Returns nullptr if Value is not a pointer to a class 
        virtual UClass* GetValueClass() const = 0;

        // Pointer to a FViewModelPropertyBase
        const FViewModelPropertyBase* Property;
    };

    // contains reflection data about single viewmodel property
    struct UNREALMVVM_API FViewModelPropertyReflection
    {
        struct FFlags
        {
            bool IsOptional : 1;
            bool HasPublicGetter : 1;
            bool HasPublicSetter : 1;
        };

        struct FBuffer
        {
            FBuffer() = default;
            FBuffer(const FBuffer& Other)
            {
                FMemory::Memcpy(&Data, &Other.Data, sizeof(Data));
            }

            TTypeCompatibleBytes<FViewModelPropertyOperations> Data;
        };

        const FViewModelPropertyOperations& GetOperations() const
        {
            return *Buffer.Data.GetTypedPtr();
        }

        const FViewModelPropertyBase* GetProperty() const
        {
            return GetOperations().Property;
        }

        FBuffer Buffer;
        int32 SizeOfValue;
        FFlags Flags;

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
