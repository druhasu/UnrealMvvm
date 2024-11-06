// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/Impl/Property/PinTraits.h"
#include "Mvvm/Impl/Property/ViewModelPropertyReflection.h"
//#include "Templates/EnableIf.h"
//#include "Templates/IsEnumClass.h"

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

        template <typename T>
        static const FViewModelPropertyReflection* FindProperty(const FName& InPropertyName)
        {
            return FindProperty(T::StaticClass(), InPropertyName);
        }

        static const FViewModelPropertyReflection* FindProperty(UClass* InViewModelClass, const FName& InPropertyName);

        static const TMap<UClass*, TArray<FViewModelPropertyReflection>>& GetAllProperties() { return ViewModelProperties; }

        template<typename TOwner, typename TValue>
        static uint8 RegisterPropertyGetter(typename TViewModelProperty<TOwner, TValue>::FPropertyGetterPtr PropertyGetterPtr);

        static void ProcessPendingRegistrations();
        static void DeleteKeptProperties();

    private:
        friend class FViewModelPropertyIterator;

        struct FUnprocessedPropertyEntry
        {
            FClassGetterPtr GetClass;
            FViewModelPropertyReflection Reflection;
        };

        static const FViewModelPropertyReflection* FindPropertyInternal(UClass* InViewModelClass, const FName& InPropertyName);
        static void GenerateReferenceTokenStream(class UClass* ViewModelClass);

        // List of properties that were not yet added to lookup table
        static TArray<FUnprocessedPropertyEntry>& GetUnprocessedProperties();

        // Map of <ViewModelClass, Properties>
        static TMap<UClass*, TArray<FViewModelPropertyReflection>> ViewModelProperties;

        // List of properties that we keep for GC (TMap and TSet properties)
        static TArray<FField*> PropertiesToKeep;
    };

}

#include "Mvvm/Impl/Property/ViewModelPropertyOperations.h"

template<typename TOwner, typename TValue>
inline uint8 UnrealMvvm_Impl::FViewModelRegistry::RegisterPropertyGetter(typename TViewModelProperty<TOwner, TValue>::FPropertyGetterPtr PropertyGetterPtr)
{
    using namespace UnrealMvvm_Impl;
    using TDecayedValue = typename TDecay<TValue>::Type;

    FUnprocessedPropertyEntry& Entry = GetUnprocessedProperties().AddDefaulted_GetRef();
    Entry.GetClass = &StaticClass<TOwner>;

    const bool IsOptional = TPinTraits<TDecayedValue>::IsOptional;
    const bool IsObject = TIsPointer<TValue>::Value && TModels<CStaticClassProvider, typename TRemoveObjectPointer<typename TRemovePointer<TValue>::Type>::Type>::Value;

    using FBaseOps    = Details::TBaseOperation<TOwner, TValue>;
    using FGetOps     = Details::TGetValueOperation<FBaseOps, TOwner, TValue, IsOptional>;
    using FSetOps     = Details::TSetValueOperation<FGetOps, TOwner, TValue, IsOptional>;
    using FAddPropOps = Details::TAddClassPropertyOperation<FSetOps, TOwner, TValue>;
    using FGetVMOps   = Details::TGetViewModelClassOperation<FAddPropOps, TOwner, TValue>;
    using FGetClassOps = Details::TGetValueClassOperation<FGetVMOps, TOwner, TValue, IsObject>;

    using FEffectiveOpsType = TViewModelPropertyOperations<FGetClassOps>;

    static_assert(sizeof(FViewModelPropertyOperations) == sizeof(FEffectiveOpsType), "Generated Operations type cannot fit into OpsBuffer");

    FViewModelPropertyReflection& Item = Entry.Reflection;

    const TViewModelProperty<TOwner, TValue>* Prop = PropertyGetterPtr();
    new (Item.OpsBuffer.Buffer.GetTypedPtr()) FEffectiveOpsType(Prop);

    Item.SizeOfValue = sizeof(TDecayedValue);

    Item.Flags.IsOptional = IsOptional;
    Item.Flags.HasPublicGetter = Prop->HasPublicGetter();
    Item.Flags.HasPublicSetter = Prop->HasPublicSetter();

#if WITH_EDITOR
    Item.PinCategoryType = TPinTraits<TDecayedValue>::PinCategoryType;
    Item.PinValueCategoryType = TPinTraits<TDecayedValue>::PinValueCategoryType;
    Item.ContainerType = TPinTraits<TDecayedValue>::PinContainerType;
    Item.GetPinSubCategoryObject = &TPinTraits<TDecayedValue>::GetSubCategoryObject;
    Item.GetPinValueSubCategoryObject = &TPinTraits<TDecayedValue>::GetValueSubCategoryObject;
#endif

    return 1;
}
