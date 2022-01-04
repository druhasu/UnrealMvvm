// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/Impl/PinTraits.h"
#include "Mvvm/Impl/ViewModelPropertyReflection.h"
#include "Templates/EnableIf.h"
#include "Templates/IsEnumClass.h"

class UClass;

template<typename TOwner, typename TValue>
class TViewModelProperty;

namespace UnrealMvvm_Impl
{

    class UNREALMVVM_API FViewModelRegistry
    {
    public:
        using ClassGetterPtr = UClass* (*)();

        template <typename T>
        static TArray<const FViewModelPropertyReflection*> GetProperties()
        {
            return GetProperties(T::StaticClass());
        }

        static TArray<const FViewModelPropertyReflection*> GetProperties(UClass* InViewModelClass);

        template <typename T>
        static const FViewModelPropertyReflection* FindProperty(const FName& InPropertyName)
        {
            return FindProperty(T::StaticClass(), InPropertyName);
        }

        static const FViewModelPropertyReflection* FindProperty(UClass* InViewModelClass, const FName& InPropertyName);

        static UClass* GetViewModelClass(UClass* ViewClass);

        static uint8 RegisterViewModelClass(ClassGetterPtr ViewClassGetter, ClassGetterPtr ViewModelClassGetter);

        template<typename TOwner, typename TValue>
        static uint8 RegisterPropertyGetter(const TViewModelProperty<TOwner, TValue>* (*PropertyGetterPtr)());

    private:
        struct FUnprocessedPropertyEntry
        {
            ClassGetterPtr GetClass;
            FViewModelPropertyReflection Reflection;
        };

        struct FUnprocessedViewModelClassEntry
        {
            ClassGetterPtr GetViewClass;
            ClassGetterPtr GetViewModelClass;
        };

        static void ProcessProperties();
        static void ProcessClasses();
        static void AppendProperties(TArray<const FViewModelPropertyReflection*>& OutArray, UClass* InViewModelClass);
        static const FViewModelPropertyReflection* FindPropertyInternal(UClass* InViewModelClass, const FName& InPropertyName);

        // Map of <ViewModelClass, Properties>
        static TMap<UClass*, TArray<FViewModelPropertyReflection>> ViewModelProperties;

        // Map of <ViewClass, ViewModelClass>
        static TMap<UClass*, UClass*> ViewModelClasses;

        // List of properties that were not yet added to lookup table
        static TArray<FUnprocessedPropertyEntry> UnprocessedProperties;

        // List of view model classes that were not yet added to lookup table
        static TArray<FUnprocessedViewModelClassEntry> UnprocessedViewModelClasses;
    };

    template <typename TOwner, typename TValue, bool IsOptional>
    struct TCopyValueFunctionHelper
    {
        static void Make(const TViewModelProperty<TOwner, TValue>* Prop, FViewModelPropertyReflection::FCopyValueFunction& OutFunction);
    };

    template<typename TOwner, typename TValue>
    inline uint8 FViewModelRegistry::RegisterPropertyGetter(const TViewModelProperty<TOwner, TValue>* (*PropertyGetterPtr)())
    {
        using TDecayedValue = typename TDecay<TValue>::Type;

        const TViewModelProperty<TOwner, TValue>* Prop = PropertyGetterPtr();

        FUnprocessedPropertyEntry& Entry = UnprocessedProperties.AddDefaulted_GetRef();
        Entry.GetClass = &StaticClass<TOwner>;

        FViewModelPropertyReflection& Item = Entry.Reflection;
        Item.Property = Prop;

        const bool IsOptional = TPinTraits<TDecayedValue>::IsOptional;
        TCopyValueFunctionHelper<TOwner, TValue, IsOptional>::Make(Prop, Item.CopyValueToMemory);
        Item.IsOptional = IsOptional;

#if WITH_EDITOR
        Item.PinCategoryType = TPinTraits<TDecayedValue>::PinCategoryType;
        Item.PinValueCategoryType = TPinTraits<TDecayedValue>::PinValueCategoryType;
        Item.ContainerType = TPinTraits<TDecayedValue>::PinContainerType;
        Item.GetPinSubCategoryObject = &TPinTraits<TDecayedValue>::GetSubCategoryObject;
        Item.GetPinValueSubCategoryObject = &TPinTraits<TDecayedValue>::GetValueSubCategoryObject;
#endif

        return 1;
    }

    template <typename TOwner, typename TValue>
    struct TCopyValueFunctionHelper<TOwner, TValue, true>
    {
        static void Make(const TViewModelProperty<TOwner, TValue>* Prop, FViewModelPropertyReflection::FCopyValueFunction& OutFunction)
        {
            using TDecayedValue = typename TDecay<TValue>::Type;

            OutFunction = [Prop](UBaseViewModel* VM, void* Dest, bool& HasValue)
            {
                TValue Value = Prop->GetValue((TOwner*)VM);
                HasValue = Value.IsSet();
                if (HasValue)
                {
                    *((TDecayedValue*)Dest) = Value.GetValue();
                }
            };
        }
    };

    template <typename TOwner, typename TValue>
    struct TCopyValueFunctionHelper<TOwner, TValue, false>
    {
        static void Make(const TViewModelProperty<TOwner, TValue>* Prop, FViewModelPropertyReflection::FCopyValueFunction& OutFunction)
        {
            using TDecayedValue = typename TDecay<TValue>::Type;

            OutFunction = [Prop](UBaseViewModel* VM, void* Dest, bool& HasValue)
            {
                *((TDecayedValue*)Dest) = Prop->GetValue((TOwner*)VM);
                HasValue = true;
            };
        }
    };
}