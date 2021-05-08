// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/ViewModelProperty.h"
#include "Mvvm/PropertyTypeSelector.h"
#include "CoreMinimal.h"
#include "BaseViewModel.generated.h"

/*
 * Base class for ViewModels
 */ 
UCLASS()
class UNREALMVVM_API UBaseViewModel : public UObject
{
    GENERATED_BODY()

public:
    DECLARE_MULTICAST_DELEGATE_OneParam(FPropertyChangedDelegate, const FViewModelPropertyBase*);
    FPropertyChangedDelegate Changed;

protected:
    void RaiseChanged(const FViewModelPropertyBase* Property)
    {
        Changed.Broadcast(Property);
    }

    template <typename T>
    T GetFieldValue(const T& Field) { return Field; }

    template <typename T>
    T* GetFieldValue(T* Field) { return Field; }

    template <typename T>
    T* GetFieldValue(const TScriptInterface<T>& Field) { return (T*)Field.GetInterface(); }

    template <typename T>
    T* GetFieldValue(const TWeakObjectPtr<T>& Field) { return Field.Get(); }
};

/*
 * Several helper macros for easier declaration of properties
 */

#define VM_PROP_PROPERTY_GETTER(Name, ValueType, GetterPtr, SetterPtr) \
    static const TViewModelProperty<ThisClass, ValueType>* Name##Property() \
    { \
        static constexpr TViewModelProperty<ThisClass, ValueType> Property = TViewModelProperty<ThisClass, ValueType> { GetterPtr, SetterPtr }; \
        return &Property; \
    }

#define VM_PROP_COMMON(ValueType, Name, GetterVisibility, SetterVisibility, GetterBody, SetterBody, FieldBody) \
GetterVisibility: \
    typename TPropertyTypeSelector<ValueType>::GetterType Get##Name() GetterBody \
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
    ValueType Name##Field;

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
    typename TPropertyTypeSelector<ValueType>::GetterType Get##Name(); \
public: \
    VM_PROP_PROPERTY_GETTER(Name, ValueType, &ThisClass::Get##Name, nullptr)
