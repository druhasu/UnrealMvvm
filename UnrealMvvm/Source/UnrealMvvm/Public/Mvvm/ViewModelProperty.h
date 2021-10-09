// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/PropertyTypeSelector.h"

/** Base non-template class for use as an Id */
class UNREALMVVM_API FViewModelPropertyBase
{
};

template<typename TOwner, typename TValue>
class TViewModelProperty : public FViewModelPropertyBase
{
public:
    using ViewModelType = TOwner;
    using ValueType = TValue;

    using GetterReturnType = typename TPropertyTypeSelector<TValue>::GetterType;
    using SetterArgumentType = typename TPropertyTypeSelector<TValue>::SetterType;

    using GetterPtr = GetterReturnType (TOwner::*) () const;
    using SetterPtr = void (TOwner::*) (SetterArgumentType);

    constexpr TViewModelProperty(GetterPtr InGetter, SetterPtr InSetter)
        : Getter(InGetter)
        , Setter(InSetter)
    {
    }

    GetterReturnType GetValue(ViewModelType* Owner) const
    {
        return (Owner->*Getter)();
    }

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
