// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

// DO NOT include this header directly!
// Use #include "Mvvm/BaseViewModel.h"

/*
 * Several helper macros for easier declaration of properties. 
 */

#define VM_PROP_PROPERTY_GETTER(Name, ValueType, GetterPtr, SetterPtr) \
    static const TViewModelProperty<ThisClass, ValueType>* Name##Property() \
    { \
        static constexpr TViewModelProperty<ThisClass, ValueType> Property = TViewModelProperty<ThisClass, ValueType> { GetterPtr, SetterPtr }; \
        return &Property; \
    }

#define VM_PROP_COMMON(ValueType, Name, GetterVisibility, SetterVisibility, GetterBody, SetterBody, FieldBody) \
GetterVisibility: \
    typename TPropertyTypeSelector<ValueType>::GetterType Get##Name() const GetterBody \
SetterVisibility: \
    void Set##Name(typename TPropertyTypeSelector<ValueType>::SetterType InNewValue) \
    SetterBody \
public: \
    VM_PROP_PROPERTY_GETTER(Name, ValueType, &ThisClass::Get##Name, &ThisClass::Set##Name) \
private: \
    FieldBody

#define VM_PROP_AUTO_GETTER(Name) \
    { return GetFieldValue( Name##Field ); }

#define VM_PROP_AUTO_SETTER(Name) \
    { \
        Name##Field = InNewValue; \
        RaiseChanged(Name##Property()); \
    }

#define VM_PROP_AUTO_FIELD(ValueType, Name) \
    typename TPropertyTypeSelector<ValueType>::FieldType Name##Field;

/*
 * Macros to declare properties with automatic backing fields
 */

/* Creates ViewModel property with manual getter and setter */
#define VM_PROP_MG_MS(ValueType, Name, GetterVisibility, SetterVisibility) \
    VM_PROP_COMMON(ValueType, Name, GetterVisibility, SetterVisibility, ;, ;, VM_PROP_AUTO_FIELD(ValueType, Name))

/* Creates ViewModel property with auto getter and manual setter */
#define VM_PROP_AG_MS(ValueType, Name, GetterVisibility, SetterVisibility) \
    VM_PROP_COMMON(ValueType, Name, GetterVisibility, SetterVisibility, VM_PROP_AUTO_GETTER(Name), ;, VM_PROP_AUTO_FIELD(ValueType, Name))

/* Creates ViewModel property with manual getter and auto setter */
#define VM_PROP_MG_AS(ValueType, Name, GetterVisibility, SetterVisibility) \
    VM_PROP_COMMON(ValueType, Name, GetterVisibility, SetterVisibility, ;, VM_PROP_AUTO_SETTER(Name), VM_PROP_AUTO_FIELD(ValueType, Name))

/* Creates ViewModel property with auto getter and setter */
#define VM_PROP_AG_AS(ValueType, Name, GetterVisibility, SetterVisibility) \
    VM_PROP_COMMON(ValueType, Name, GetterVisibility, SetterVisibility, VM_PROP_AUTO_GETTER(Name), VM_PROP_AUTO_SETTER(Name), VM_PROP_AUTO_FIELD(ValueType, Name))

/*
 * Macros to declare properties without backing fields
 */

/* Creates ViewModel property with manual getter and setter and no backing field */
#define VM_PROP_MG_MS_NF(ValueType, Name, GetterVisibility, SetterVisibility) \
    VM_PROP_COMMON(ValueType, Name, GetterVisibility, SetterVisibility, ;, ;, )

/* Creates ViewModel property with auto getter and manual setter and no backing field */
#define VM_PROP_AG_MS_NF(ValueType, Name, GetterVisibility, SetterVisibility) \
    VM_PROP_COMMON(ValueType, Name, GetterVisibility, SetterVisibility, VM_PROP_AUTO_GETTER(Name), ;, )

/* Creates ViewModel property with manual getter and auto setter and no backing field */
#define VM_PROP_MG_AS_NF(ValueType, Name, GetterVisibility, SetterVisibility) \
    VM_PROP_COMMON(ValueType, Name, GetterVisibility, SetterVisibility, ;, VM_PROP_AUTO_SETTER(Name), )

/* Creates ViewModel property with auto getter and setter and no backing field */
#define VM_PROP_AG_AS_NF(ValueType, Name, GetterVisibility, SetterVisibility) \
    VM_PROP_COMMON(ValueType, Name, GetterVisibility, SetterVisibility, VM_PROP_AUTO_GETTER(Name), VM_PROP_AUTO_SETTER(Name), )

/* Creates ViewModel property with manual getter and no backing field */
#define VM_PROP_MG_NF(ValueType, Name, GetterVisibility) \
GetterVisibility: \
    typename TPropertyTypeSelector<ValueType>::GetterType Get##Name() const; \
public: \
    VM_PROP_PROPERTY_GETTER(Name, ValueType, &ThisClass::Get##Name, nullptr)