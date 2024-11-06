// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/Impl/Property/PropertyTypeSelector.h"
#include "Mvvm/Impl/Property/ViewModelPropertyNamesCache.h"

/** Base non-template class for use as an Id */
class UNREALMVVM_API FViewModelPropertyBase
{
public:
    enum class EAccessorVisibility { V_public, V_protected, V_private };

    constexpr FViewModelPropertyBase(const ANSICHAR* InName, int32 InFieldOffset, EAccessorVisibility GetterVisibility, EAccessorVisibility SetterVisibility, bool bInHasSetter)
        : Name(InName)
        , FieldOffset(InFieldOffset)
        , bGetterIsPublic(GetterVisibility == EAccessorVisibility::V_public)
        , bSetterIsPublic(SetterVisibility == EAccessorVisibility::V_public)
        , bHasSetter(bInHasSetter)
    {
    }

    /* Returns Name of a property */
    FName GetName() const
    {
        return UnrealMvvm_Impl::FViewModelPropertyNamesCache::GetPropertyName(this);
    }

    /* Returns Name of UFunction that is called when this property changes */
    FName GetLegacyCallbackName() const
    {
        return UnrealMvvm_Impl::FViewModelPropertyNamesCache::GetPropertyCallbackName(this);
    }

    /* Returns Offset of a backing field from beginning of owning object */
    int32 GetFieldOffset() const
    {
        return FieldOffset;
    }

    /* Returns whether this property has Getter with public visibility */
    bool HasPublicGetter() const
    {
        return bGetterIsPublic;
    }

    /* Returns whether this property has Setter with public visibility */
    bool HasPublicSetter() const
    {
        return bSetterIsPublic;
    }

    /* Returns whether this property has Setter or is Getter-only */
    bool HasSetter() const
    {
        return bHasSetter;
    }

private:
    friend class UnrealMvvm_Impl::FViewModelPropertyNamesCache;

    const ANSICHAR* Name;

    int32 FieldOffset;
    uint8 bGetterIsPublic : 1;
    uint8 bSetterIsPublic : 1;
    uint8 bHasSetter : 1;
};

/*
 * Property of a ViewModel
 */
template<typename TOwner, typename TValue>
class TViewModelProperty : public FViewModelPropertyBase
{
public:
    using FViewModelType = TOwner;
    using FValueType = TValue;

    using FGetterReturnType = typename UnrealMvvm_Impl::TPropertyTypeSelector<TValue>::GetterType;
    using FSetterArgumentType = typename UnrealMvvm_Impl::TPropertyTypeSelector<TValue>::SetterType;

    using FPropertyGetterPtr = const TViewModelProperty<TOwner, TValue>* (*)();
    using FGetterPtr = FGetterReturnType (TOwner::*) () const;
    using FSetterPtr = void (TOwner::*) (FSetterArgumentType);

    constexpr TViewModelProperty(FGetterPtr InGetter, FSetterPtr InSetter, int32 InFieldOffset, EAccessorVisibility GetterVisibility, EAccessorVisibility SetterVisibility, const ANSICHAR* InName)
        : FViewModelPropertyBase(InName, InFieldOffset, GetterVisibility, SetterVisibility, InSetter != nullptr)
        , Getter(InGetter)
        , Setter(InSetter)
    {
    }

    /* Returns value of this property from given ViewModel */
    FGetterReturnType GetValue(FViewModelType* Owner) const
    {
        return (Owner->*Getter)();
    }

    /* Sets value of this property to given ViewModel */
    void SetValue(FViewModelType* Owner, FSetterArgumentType Value) const
    {
        if (Setter)
        {
            (Owner->*Setter)(Value);
        }
    }

private:
    FGetterPtr Getter;
    FSetterPtr Setter; // this variable MUST BE the last one due to a bug in MSVC compiler
};

/* Helper class that registers a property into reflection system */
template
<
    typename TOwner,
    typename TValue,
    typename TViewModelProperty<TOwner, TValue>::FPropertyGetterPtr PropertyGetterPtr
>
class TViewModelPropertyRegistered : public TViewModelProperty<TOwner, TValue>
{
    using Super = TViewModelProperty<TOwner, TValue>;

public:
    constexpr TViewModelPropertyRegistered(typename Super::FGetterPtr InGetter, typename Super::FSetterPtr InSetter, int32 InFieldOffset, typename Super::EAccessorVisibility GetterVisibility, typename Super::EAccessorVisibility SetterVisibility, const ANSICHAR* InName)
        : Super(InGetter, InSetter, InFieldOffset, GetterVisibility, SetterVisibility, InName)
    {
    }

public:
    static const uint8 Registered;
};

#include "Mvvm/Impl/Property/ViewModelRegistry.h"

template
<
    typename TOwner,
    typename TValue,
    typename TViewModelProperty<TOwner, TValue>::FPropertyGetterPtr PropertyGetterPtr
>
const uint8 TViewModelPropertyRegistered<TOwner, TValue, PropertyGetterPtr>::Registered = UnrealMvvm_Impl::FViewModelRegistry::RegisterPropertyGetter<TOwner, TValue>(PropertyGetterPtr);
