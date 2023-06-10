// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/Impl/PropertyTypeSelector.h"
#include "Mvvm/Impl/ViewModelPropertyNamesCache.h"

/** Base non-template class for use as an Id */
class UNREALMVVM_API FViewModelPropertyBase
{
public:
    constexpr FViewModelPropertyBase(const ANSICHAR* InName)
        : Name(InName)
    {
    }

    /* Returns Name of a property */
    FName GetName() const
    {
        return UnrealMvvm_Impl::FViewModelPropertyNamesCache::GetPropertyName(this);
    }

    /* Returns Name of UFunction that is called when this property changes */
    FName GetCallbackName() const
    {
        return UnrealMvvm_Impl::FViewModelPropertyNamesCache::GetPropertyCallbackName(this);
    }

private:
    friend class UnrealMvvm_Impl::FViewModelPropertyNamesCache;

    const ANSICHAR* Name;
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

    enum class EAccessorVisibility { V_public, V_protected, V_private };

    constexpr TViewModelProperty(FGetterPtr InGetter, FSetterPtr InSetter, int32 InFieldOffset, EAccessorVisibility GetterVisibility, EAccessorVisibility SetterVisibility, const ANSICHAR* InName)
        : FViewModelPropertyBase(InName)
        , Getter(InGetter)
        , Setter(InSetter)
        , FieldOffset(InFieldOffset)
        , bGetterIsPublic(GetterVisibility == EAccessorVisibility::V_public)
        , bSetterIsPublic(SetterVisibility == EAccessorVisibility::V_public)
    {
    }

    /* Returns value of this property from given ViewModel */
    FGetterReturnType GetValue(FViewModelType* Owner) const
    {
        return (Owner->*Getter)();
    }

    bool HasPublicGetter() const
    {
        return bGetterIsPublic;
    }

    /* Sets value of this property to given ViewModel */
    void SetValue(FViewModelType* Owner, FSetterArgumentType Value) const
    {
        if (Setter)
        {
            (Owner->*Setter)(Value);
        }
    }

    bool HasPublicSetter() const
    {
        return bSetterIsPublic;
    }

    int32 GetFieldOffset() const
    {
        return FieldOffset;
    }

private:
    FGetterPtr Getter;
    FSetterPtr Setter;
    int32 FieldOffset;
    uint8 bGetterIsPublic : 1;
    uint8 bSetterIsPublic : 1;
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

#include "Mvvm/Impl/ViewModelRegistry.h"

template
<
    typename TOwner,
    typename TValue,
    typename TViewModelProperty<TOwner, TValue>::FPropertyGetterPtr PropertyGetterPtr
>
const uint8 TViewModelPropertyRegistered<TOwner, TValue, PropertyGetterPtr>::Registered = UnrealMvvm_Impl::FViewModelRegistry::RegisterPropertyGetter<TOwner, TValue>(PropertyGetterPtr);