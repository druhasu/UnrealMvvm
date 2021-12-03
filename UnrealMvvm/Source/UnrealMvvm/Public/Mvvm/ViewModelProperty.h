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
    using ViewModelType = TOwner;
    using ValueType = TValue;

    using GetterReturnType = typename UnrealMvvm_Impl::TPropertyTypeSelector<TValue>::GetterType;
    using SetterArgumentType = typename UnrealMvvm_Impl::TPropertyTypeSelector<TValue>::SetterType;

    using GetterPtr = GetterReturnType (TOwner::*) () const;
    using SetterPtr = void (TOwner::*) (SetterArgumentType);

    constexpr TViewModelProperty(GetterPtr InGetter, SetterPtr InSetter, const ANSICHAR* InName)
        : FViewModelPropertyBase(InName)
        , Getter(InGetter)
        , Setter(InSetter)
    {
    }

    /* Returns value of this property from given ViewModel */
    GetterReturnType GetValue(ViewModelType* Owner) const
    {
        return (Owner->*Getter)();
    }

    /* Sets value of this property to given ViewModel */
    void SetValue(ViewModelType* Owner, SetterArgumentType Value) const
    {
        if (Setter)
        {
            (Owner->*Setter)(Value);
        }
    }

private:
    GetterPtr Getter;
    SetterPtr Setter;
};

/* Helper class that registers a property into reflection system */
template<typename TOwner, typename TValue, const TViewModelProperty<TOwner, TValue>*(*PropertyGetterPtr)()>
class TViewModelPropertyRegistered : public TViewModelProperty<TOwner, TValue>
{
    using Super = TViewModelProperty<TOwner, TValue>;

public:
    constexpr TViewModelPropertyRegistered(typename Super::GetterPtr InGetter, typename Super::SetterPtr InSetter, const ANSICHAR* InName)
        : Super(InGetter, InSetter, InName)
    {
    }

public:
    static const uint8 Registered;
};

#include "Mvvm/Impl/ViewModelRegistry.h"

template<typename TOwner, typename TValue, const TViewModelProperty<TOwner, TValue>* (*PropertyGetterPtr)()>
const uint8 TViewModelPropertyRegistered<TOwner, TValue, PropertyGetterPtr>::Registered = UnrealMvvm_Impl::FViewModelRegistry::RegisterPropertyGetter<TOwner, TValue>(PropertyGetterPtr);