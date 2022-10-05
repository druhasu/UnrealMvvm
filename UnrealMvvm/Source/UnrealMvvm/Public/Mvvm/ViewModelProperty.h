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

    constexpr TViewModelProperty(FGetterPtr InGetter, FSetterPtr InSetter, int32 InFieldOffset, const ANSICHAR* InName)
        : FViewModelPropertyBase(InName)
        , Getter(InGetter)
        , Setter(InSetter)
        , FieldOffset(InFieldOffset)
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

    int32 GetFieldOffset() const
    {
        return FieldOffset;
    }

private:
    FGetterPtr Getter;
    FSetterPtr Setter;
    int32 FieldOffset;
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
    constexpr TViewModelPropertyRegistered(typename Super::FGetterPtr InGetter, typename Super::FSetterPtr InSetter, int32 InFieldOffset, const ANSICHAR* InName)
        : Super(InGetter, InSetter, InFieldOffset, InName)
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