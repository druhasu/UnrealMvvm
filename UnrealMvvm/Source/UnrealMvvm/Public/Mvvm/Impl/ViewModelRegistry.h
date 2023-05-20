// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/Impl/PinTraits.h"
#include "Mvvm/Impl/ViewModelPropertyReflection.h"
#include "Templates/EnableIf.h"
#include "Templates/IsEnumClass.h"

class UClass;
class UUserWidget;
class UBaseViewModel;

template<typename TOwner, typename TValue>
class TViewModelProperty;

namespace UnrealMvvm_Impl
{

    class UNREALMVVM_API FViewModelRegistry
    {
    public:
        using FClassGetterPtr = UClass * (*)();
        using FViewModelSetterPtr = void (*)(UUserWidget&, UBaseViewModel*);

        template <typename T>
        static const FViewModelPropertyReflection* FindProperty(const FName& InPropertyName)
        {
            return FindProperty(T::StaticClass(), InPropertyName);
        }

        static const FViewModelPropertyReflection* FindProperty(UClass* InViewModelClass, const FName& InPropertyName);

        static UClass* GetViewModelClass(UClass* ViewClass);

        static FViewModelSetterPtr GetViewModelSetter(UClass* ViewClass);

        static const TMap<UClass*, TArray<FViewModelPropertyReflection>>& GetAllProperties() { return ViewModelProperties; }

        static uint8 RegisterViewClass(FClassGetterPtr ViewClassGetter, FClassGetterPtr ViewModelClassGetter, FViewModelSetterPtr ViewModelSetter);
        static void RegisterViewClass(UClass* ViewClass, UClass* ViewModelClass);

        template<typename TOwner, typename TValue>
        static uint8 RegisterPropertyGetter(typename TViewModelProperty<TOwner, TValue>::FPropertyGetterPtr PropertyGetterPtr);

        static void ProcessPendingRegistrations();
        static void DeleteKeptProperties();

#if WITH_EDITOR
        DECLARE_MULTICAST_DELEGATE_TwoParams(FViewModelClassChanged, UClass* /*ViewClass*/, UClass* /*ViewModelClass*/);
        static FViewModelClassChanged ViewClassChanged;
#endif

    private:
        friend class FViewModelPropertyIterator;

        struct FUnprocessedPropertyEntry
        {
            FClassGetterPtr GetClass;
            FViewModelPropertyReflection Reflection;
        };

        struct FUnprocessedViewModelClassEntry
        {
            FClassGetterPtr GetViewClass;
            FClassGetterPtr GetViewModelClass;
            FViewModelSetterPtr ViewModelSetter;
        };

        static const FViewModelPropertyReflection* FindPropertyInternal(UClass* InViewModelClass, const FName& InPropertyName);
        static void GenerateReferenceTokenStream(class UClass* ViewModelClass);

        // Map of <ViewModelClass, Properties>
        static TMap<UClass*, TArray<FViewModelPropertyReflection>> ViewModelProperties;

        // Map of <ViewClass, ViewModelClass>
        static TMap<UClass*, UClass*> ViewModelClasses;

        // Map of <ViewClass, Setter Function>
        static TMap<UClass*, FViewModelSetterPtr> ViewModelSetters;

        // List of properties that were not yet added to lookup table
        static TArray<FUnprocessedPropertyEntry> UnprocessedProperties;

        // List of view model classes that were not yet added to lookup table
        static TArray<FUnprocessedViewModelClassEntry> UnprocessedViewModelClasses;

        // List of properties that we keep for GC (TMap and TSet properties)
        static TArray<FField*> PropertiesToKeep;
    };

}

#include "Mvvm/Impl/ViewModelPropertyOperations.h"

template<typename TOwner, typename TValue>
inline uint8 UnrealMvvm_Impl::FViewModelRegistry::RegisterPropertyGetter(typename TViewModelProperty<TOwner, TValue>::FPropertyGetterPtr PropertyGetterPtr)
{
    using namespace UnrealMvvm_Impl;
    using TDecayedValue = typename TDecay<TValue>::Type;

    FUnprocessedPropertyEntry& Entry = UnprocessedProperties.AddDefaulted_GetRef();
    Entry.GetClass = &StaticClass<TOwner>;

    const bool IsOptional = TPinTraits<TDecayedValue>::IsOptional;

    using FBaseOps    = Details::TBaseOperation<TOwner, TValue>;
    using FCopyOps    = Details::TCopyValueOperation<FBaseOps, TOwner, TValue, IsOptional>;
    using FAddPropOps = Details::TAddClassPropertyOperation<FCopyOps, TOwner, TValue>;
    using FGetVMOps   = Details::TGetViewModelClassOperation<FAddPropOps, TOwner, TValue>;

    using FEffectiveOpsType = TViewModelPropertyOperations<FGetVMOps>;

    static_assert(sizeof(FViewModelPropertyOperations) == sizeof(FEffectiveOpsType), "Generated Operations type cannot fit into OpsBuffer");

    FViewModelPropertyReflection& Item = Entry.Reflection;

    const TViewModelProperty<TOwner, TValue>* Prop = PropertyGetterPtr();
    new (Item.OpsBuffer.Buffer.GetTypedPtr()) FEffectiveOpsType(Prop);

    Item.Flags.IsOptional = IsOptional;

#if WITH_EDITOR
    Item.PinCategoryType = TPinTraits<TDecayedValue>::PinCategoryType;
    Item.PinValueCategoryType = TPinTraits<TDecayedValue>::PinValueCategoryType;
    Item.ContainerType = TPinTraits<TDecayedValue>::PinContainerType;
    Item.GetPinSubCategoryObject = &TPinTraits<TDecayedValue>::GetSubCategoryObject;
    Item.GetPinValueSubCategoryObject = &TPinTraits<TDecayedValue>::GetValueSubCategoryObject;
#endif

    return 1;
}