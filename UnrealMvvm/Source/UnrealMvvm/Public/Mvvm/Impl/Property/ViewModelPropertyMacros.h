// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

// DO NOT include this header directly!
// Use #include "Mvvm/BaseViewModel.h"

/*
 * Several helper macros for easier declaration of properties. 
 */

// shorthand for remove parentheses
#define UMVVM_IMPL_RP PREPROCESSOR_REMOVE_OPTIONAL_PARENS

// Calls Macro usign Args. Args are expected to be inside parantheses
#define UMVVM_IMPL_INDIRECT_CALL(Macro, Args) Macro Args

// UMVVM_IMPL_NARGS - returns number of arguments passed
#if defined _MSC_VER && (!defined(_MSVC_TRADITIONAL) || _MSVC_TRADITIONAL) // Microsoft compilers without /Zc:preprocessor

    #define UMVVM_IMPL_EXPAND(x) x
    #define UMVVM_IMPL_COUNTER(_1, _2, _3, _4, _5, VAL, ...) VAL
    #define UMVVM_IMPL_EXPANDER(...) UMVVM_IMPL_EXPAND(UMVVM_IMPL_COUNTER(__VA_ARGS__, 4, 3, 2, 1, 0))

    #define UMVVM_IMPL_AUGMENTER(...) unused, __VA_ARGS__
    #define UMVVM_IMPL_NARGS(...) UMVVM_IMPL_EXPANDER(UMVVM_IMPL_AUGMENTER(__VA_ARGS__))

#else // All other compilers

    #define UMVVM_IMPL_COUNTER(_0, _1, _2, _3, _4, _5, N, ...) N
    #define UMVVM_IMPL_NARGS(...) UMVVM_IMPL_COUNTER(0, ## __VA_ARGS__, 5, 4, 3, 2, 1, 0)

#endif

// creates PropertyGetter method with provided Getter and Setter visibility
#define UMVVM_IMPL_PROP_PROPERTY_GETTER_2(Name, ValueType, GetterPtr, SetterPtr, FieldOffset, GetterVisibility, SetterVisibility) \
    using F##Name##Property = TViewModelProperty<ThisClass, UMVVM_IMPL_RP(ValueType)>; \
    static const TViewModelProperty<ThisClass, UMVVM_IMPL_RP(ValueType)>* Name##Property() \
    { \
        return &Name##PropertyValue; \
    } \
private: \
    static F##Name##Property Name##PropertyValue;

// creates PropertyGetter method with provided Setter visibility and default Getter visibility (public)
#define UMVVM_IMPL_PROP_PROPERTY_GETTER_1(Name, ValueType, GetterPtr, SetterPtr, FieldOffset, SetterVisibility) \
    UMVVM_IMPL_PROP_PROPERTY_GETTER_2(Name, ValueType, GetterPtr, SetterPtr, FieldOffset, public, SetterVisibility)

// creates PropertyGetter method with default Getter and Setter visibility (public, private)
#define UMVVM_IMPL_PROP_PROPERTY_GETTER_0(Name, ValueType, GetterPtr, SetterPtr, FieldOffset) \
    UMVVM_IMPL_PROP_PROPERTY_GETTER_2(Name, ValueType, GetterPtr, SetterPtr, FieldOffset, public, private)



// creates common methods for property with provided Getter and Setter visibility
#define UMVVM_IMPL_PROP_COMMON_2(ValueType, Name, GetterBody, SetterBody, FieldBody, GetterVisibility, SetterVisibility) \
GetterVisibility: \
    typename UnrealMvvm_Impl::TPropertyTypeSelector< UMVVM_IMPL_RP(ValueType) >::GetterType Get##Name() const GetterBody \
SetterVisibility: \
    void Set##Name(typename UnrealMvvm_Impl::TPropertyTypeSelector< UMVVM_IMPL_RP(ValueType) >::SetterType InNewValue) \
    SetterBody \
public: \
    UMVVM_IMPL_PROP_PROPERTY_GETTER_2(Name, ValueType, &ThisClass::Get##Name, &ThisClass::Set##Name, STRUCT_OFFSET(ThisClass, Name##Field), GetterVisibility, SetterVisibility) \
private: \
    UMVVM_IMPL_RP(FieldBody)

// creates common methods for property with provided Setter visibility and default Getter visibility (public)
#define UMVVM_IMPL_PROP_COMMON_1(ValueType, Name, GetterBody, SetterBody, FieldBody, SetterVisibility) \
    UMVVM_IMPL_PROP_COMMON_2(ValueType, Name, GetterBody, SetterBody, FieldBody, public, SetterVisibility)

// creates common methods for property with default Getter and Setter visibility (public, private)
#define UMVVM_IMPL_PROP_COMMON_0(ValueType, Name, GetterBody, SetterBody, FieldBody) \
    UMVVM_IMPL_PROP_COMMON_2(ValueType, Name, GetterBody, SetterBody, FieldBody, public, private)



// creates common methods for property with provided Getter and Setter visibility (no backing field)
#define UMVVM_IMPL_PROP_COMMON_NF_2(ValueType, Name, GetterBody, SetterBody, GetterVisibility, SetterVisibility) \
GetterVisibility: \
    typename UnrealMvvm_Impl::TPropertyTypeSelector< UMVVM_IMPL_RP(ValueType) >::GetterType Get##Name() const GetterBody \
SetterVisibility: \
    void Set##Name(typename UnrealMvvm_Impl::TPropertyTypeSelector< UMVVM_IMPL_RP(ValueType) >::SetterType InNewValue) \
    SetterBody \
public: \
    UMVVM_IMPL_PROP_PROPERTY_GETTER_2(Name, ValueType, &ThisClass::Get##Name, &ThisClass::Set##Name, 0, GetterVisibility, SetterVisibility)

// creates common methods for property with provided Setter visibility and default Getter visibility (public) (no backing field)
#define UMVVM_IMPL_PROP_COMMON_NF_1(ValueType, Name, GetterBody, SetterBody, SetterVisibility) \
    UMVVM_IMPL_PROP_COMMON_NF_2(ValueType, Name, GetterBody, SetterBody, public, SetterVisibility) \

// creates common methods for property with default Getter and Setter visibility (public, private) (no backing field)
#define UMVVM_IMPL_PROP_COMMON_NF_0(ValueType, Name, GetterBody, SetterBody) \
    UMVVM_IMPL_PROP_COMMON_NF_2(ValueType, Name, GetterBody, SetterBody, public, private) \



// creates common methods for property with manual Getter and provided visibility (no backing field)
#define UMVVM_IMPL_PROP_MG_NF_1(ValueType, Name, GetterVisibility) \
public: \
    UMVVM_IMPL_PROP_PROPERTY_GETTER_2(Name, ValueType, &ThisClass::Get##Name, nullptr, 0, GetterVisibility, private) \
GetterVisibility: \
    typename UnrealMvvm_Impl::TPropertyTypeSelector<UMVVM_IMPL_RP(ValueType)>::GetterType Get##Name() const

// creates common methods for property with manual Getter and default visibility (public) (no backing field)
#define UMVVM_IMPL_PROP_MG_NF_0(ValueType, Name) \
    UMVVM_IMPL_PROP_MG_NF_1(ValueType, Name, public)


// creates body for automatic Getter method
#define UMVVM_IMPL_PROP_AUTO_GETTER(Name) \
    { return Name##Field; }



// creates body for automatic Setter method
#define UMVVM_IMPL_PROP_AUTO_SETTER(Name) \
    { \
        if (TrySetValue(Name##Field, InNewValue)) \
        { \
            RaiseChanged(Name##Property()); \
        } \
    }



// creates automatic Field
#define UMVVM_IMPL_PROP_AUTO_FIELD(ValueType, Name) \
    typename UnrealMvvm_Impl::TPropertyTypeSelector< UMVVM_IMPL_RP(ValueType) >::FieldType Name##Field


/*
 * By default all properties have public Getter and private Setter. If you need to change it, pass desired visibility after Name parameter.
 * You may specify either only Setter visibility (if you pass single parameter) or both (if you pass two parameters)
 * Properties created with VM_PROP_MG_NF only accept single parameter, because they may not have a Setter
 * 
 * Examples:
 *   VM_PROP_AG_AS(int32, MyProperty); // use default Getter and Setter visibility
 *   VM_PROP_AG_AS(int32, MyProperty, public); // use default Getter visibility and make Setter public
 *   VM_PROP_AG_AS(int32, MyProperty, protected, protected); // make both Getter and Setter protected
 * 
 *   VM_PROP_MG_NF(int32, MyProperty); // use default Getter visibility
 *   VM_PROP_MG_NF(int32, MyProperty, protected); // make Getter protected
 */


/*
 * Macros to declare properties with automatic backing fields
 */

/*
 * Creates ViewModel property with manual getter and setter
 * Pass optional Getter and Setter visibility via VA_ARGS
 */
#define VM_PROP_MG_MS(ValueType, Name, ... /* GetterVisibility, SetterVisibility */) \
    UMVVM_IMPL_INDIRECT_CALL( \
        PREPROCESSOR_JOIN(UMVVM_IMPL_PROP_COMMON_, UMVVM_IMPL_NARGS(__VA_ARGS__)), \
        (ValueType, Name, ;, ;, (UMVVM_IMPL_PROP_AUTO_FIELD(UMVVM_IMPL_RP(ValueType), Name)), ##__VA_ARGS__) \
    )

/*
 * Creates ViewModel property with auto getter and manual setter
 * Pass optional Getter and Setter visibility via VA_ARGS
 */
#define VM_PROP_AG_MS(ValueType, Name, ... /* GetterVisibility, SetterVisibility */) \
    UMVVM_IMPL_INDIRECT_CALL( \
        PREPROCESSOR_JOIN(UMVVM_IMPL_PROP_COMMON_, UMVVM_IMPL_NARGS(__VA_ARGS__)), \
        (ValueType, Name, UMVVM_IMPL_PROP_AUTO_GETTER(Name), ;, (UMVVM_IMPL_PROP_AUTO_FIELD(UMVVM_IMPL_RP(ValueType), Name)), ##__VA_ARGS__) \
    )

/*
 * Creates ViewModel property with manual getter and auto setter
 * Pass optional Getter and Setter visibility via VA_ARGS
 */
#define VM_PROP_MG_AS(ValueType, Name, ... /* GetterVisibility, SetterVisibility */) \
    UMVVM_IMPL_INDIRECT_CALL( \
        PREPROCESSOR_JOIN(UMVVM_IMPL_PROP_COMMON_, UMVVM_IMPL_NARGS(__VA_ARGS__)), \
        (ValueType, Name, ;, UMVVM_IMPL_PROP_AUTO_SETTER(Name), (UMVVM_IMPL_PROP_AUTO_FIELD(UMVVM_IMPL_RP(ValueType), Name)), ##__VA_ARGS__) \
    )

/*
 * Creates ViewModel property with auto getter and setter
 * Pass optional Getter and Setter visibility via VA_ARGS
 */
#define VM_PROP_AG_AS(ValueType, Name, ... /* GetterVisibility, SetterVisibility */) \
    UMVVM_IMPL_INDIRECT_CALL( \
        PREPROCESSOR_JOIN(UMVVM_IMPL_PROP_COMMON_, UMVVM_IMPL_NARGS(__VA_ARGS__)), \
        (ValueType, Name, UMVVM_IMPL_PROP_AUTO_GETTER(Name), UMVVM_IMPL_PROP_AUTO_SETTER(Name), (UMVVM_IMPL_PROP_AUTO_FIELD(ValueType, Name)), ##__VA_ARGS__) \
    )

/*
 * Macros to declare properties without backing fields
 */

/*
 * Creates ViewModel property with manual getter and setter and no backing field
 * Pass optional Getter and Setter visibility via VA_ARGS
 */
#define VM_PROP_MG_MS_NF(ValueType, Name, ... /* GetterVisibility, SetterVisibility */) \
    UMVVM_IMPL_INDIRECT_CALL( \
        PREPROCESSOR_JOIN(UMVVM_IMPL_PROP_COMMON_NF_, UMVVM_IMPL_NARGS(__VA_ARGS__)), \
        (ValueType, Name, ;, ;, ##__VA_ARGS__) \
    )

/*
 * Creates ViewModel property with auto getter and manual setter and no backing field
 * Pass optional Getter and Setter visibility via VA_ARGS
 */
#define VM_PROP_AG_MS_NF(ValueType, Name, ... /* GetterVisibility, SetterVisibility */) \
    UMVVM_IMPL_INDIRECT_CALL( \
        PREPROCESSOR_JOIN(UMVVM_IMPL_PROP_COMMON_NF_, UMVVM_IMPL_NARGS(__VA_ARGS__)), \
        (ValueType, Name, UMVVM_IMPL_PROP_AUTO_GETTER(Name), ;, ##__VA_ARGS__) \
    )

/*
 * Creates ViewModel property with manual getter and auto setter and no backing field
 * Pass optional Getter and Setter visibility via VA_ARGS
 */
#define VM_PROP_MG_AS_NF(ValueType, Name, ... /* GetterVisibility, SetterVisibility */) \
    UMVVM_IMPL_INDIRECT_CALL( \
        PREPROCESSOR_JOIN(UMVVM_IMPL_PROP_COMMON_NF_, UMVVM_IMPL_NARGS(__VA_ARGS__)), \
        (ValueType, Name, ;, UMVVM_IMPL_PROP_AUTO_SETTER(Name), ##__VA_ARGS__) \
    )

/* 
 * Creates ViewModel property with auto getter and setter and no backing field
 * Pass optional Getter and Setter visibility via VA_ARGS
 */
#define VM_PROP_AG_AS_NF(ValueType, Name, ... /* GetterVisibility, SetterVisibility */) \
    UMVVM_IMPL_INDIRECT_CALL( \
        PREPROCESSOR_JOIN(UMVVM_IMPL_PROP_COMMON_NF_, UMVVM_IMPL_NARGS(__VA_ARGS__)), \
        (ValueType, Name, UMVVM_IMPL_PROP_AUTO_GETTER(Name), UMVVM_IMPL_PROP_AUTO_SETTER(Name), ##__VA_ARGS__) \
    )

/*
 * Creates ViewModel property with manual getter and no backing field
 * Pass optional Getter visibility via VA_ARGS
 */
#define VM_PROP_MG_NF(ValueType, Name, ... /* GetterVisibility */) \
    UMVVM_IMPL_INDIRECT_CALL( \
        PREPROCESSOR_JOIN(UMVVM_IMPL_PROP_MG_NF_, UMVVM_IMPL_NARGS(__VA_ARGS__)), \
        (ValueType, Name, ##__VA_ARGS__) \
    )

