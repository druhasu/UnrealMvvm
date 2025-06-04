// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/Impl/Property/PinTraits.h"
#include "Mvvm/Impl/Property/ViewModelPropertyReflection.h"

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
        static void RegisterProperty(const TViewModelProperty<TOwner, TValue>* Prop, const ANSICHAR* InName);

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
inline void UnrealMvvm_Impl::FViewModelRegistry::RegisterProperty(const TViewModelProperty<TOwner, TValue>* Prop, const ANSICHAR* InName)
{
    using namespace UnrealMvvm_Impl;
    using TDecayedValue = typename TDecay<TValue>::Type;

    FUnprocessedPropertyEntry& Entry = GetUnprocessedProperties().AddDefaulted_GetRef();
    Entry.GetClass = &StaticClass<TOwner>;

    // emplace FName into the property object
    FViewModelPropertyBase* MutablePropertyPtr = const_cast<FViewModelPropertyBase*>((const FViewModelPropertyBase*)Prop);
    *(FName*)MutablePropertyPtr->NameData = FName(InName);

    const bool IsOptional = TPinTraits<TDecayedValue>::IsOptional;
    const bool IsObject = TModels<CStaticClassProvider, typename TRemoveObjectPointer<std::remove_pointer_t<TValue>>::Type>::Value;

    using FBaseOps    = Details::TBaseOperation<TOwner, TValue>;
    using FGetOps     = Details::TGetValueOperation<FBaseOps, TOwner, TValue, IsOptional>;
    using FSetOps     = Details::TSetValueOperation<FGetOps, TOwner, TValue, IsOptional>;
    using FAddPropOps = Details::TAddClassPropertyOperation<FSetOps, TOwner, TValue>;
    using FGetVMOps   = Details::TGetViewModelClassOperation<FAddPropOps, TOwner, TValue>;
    using FGetClassOps = Details::TGetValueClassOperation<FGetVMOps, TOwner, TValue, IsObject>;

    using FEffectiveOpsType = TViewModelPropertyOperations<FGetClassOps>;

    static_assert(sizeof(FViewModelPropertyOperations) == sizeof(FEffectiveOpsType), "Generated Operations type cannot fit into OpsBuffer");

    FViewModelPropertyReflection& Item = Entry.Reflection;

    new (Item.Buffer.Data.GetTypedPtr()) FEffectiveOpsType(Prop);

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
}
