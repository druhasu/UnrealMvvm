// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

/*
 * Base struct that describe traits of a type if it is used as value of ViewModel Property
 * This struct uses same idea as TStructOpsTypeTraitsBase2
 */
template <typename TPropertyValueType>
struct TViewModelPropertyTypeTraitsBase
{
    enum
    {
        WithSetterArgumentByValue       = false,            // Forces generated setter method to accept argument By Value rather than By Reference
        WithSetterComparison            = true,             // Defines whether we need to perform comparison between existing value and new value in property Setter
    };
};

/*
 * Specialization of TViewModelPropertyTypeTraitsBase for each type
 * If you need to override some settings, make specialization of this struct for your custom type
 */
template <typename TPropertyValueType>
struct TViewModelPropertyTypeTraits : public TViewModelPropertyTypeTraitsBase<TPropertyValueType>
{
};
