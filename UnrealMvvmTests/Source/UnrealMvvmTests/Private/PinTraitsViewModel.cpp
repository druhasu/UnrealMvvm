// Copyright Andrei Sudarikov. All Rights Reserved.

#include "PinTraitsViewModel.h"
#include "Mvvm/Impl/Property/ViewModelRegistry.h"

#define VM_PROP_PIN_TRAITS_TEST_IMPL_SINGLE(ValueType, Name) \
    UPinTraitsViewModel::F##Name##Property UPinTraitsViewModel::Name##PropertyValue = { &UPinTraitsViewModel::Get##Name, &UPinTraitsViewModel::Set##Name, STRUCT_OFFSET(UPinTraitsViewModel, Name##Field), FViewModelPropertyBase::EAccessorVisibility::V_public, FViewModelPropertyBase::EAccessorVisibility::V_public }

#define VM_PROP_PIN_TRAITS_TEST_REGISTER_SINGLE(Name) \
    UnrealMvvm_Impl::FViewModelRegistry::RegisterProperty(UPinTraitsViewModel::Name##Property(), #Name);

#define VM_PROP_PIN_TRAITS_TEST_IMPL(ValueType, Name) \
    VM_PROP_PIN_TRAITS_TEST_IMPL_SINGLE(ValueType, Name); \
    VM_PROP_PIN_TRAITS_TEST_IMPL_SINGLE(ValueType, Name##Array); \
    VM_PROP_PIN_TRAITS_TEST_IMPL_SINGLE(ValueType, Name##Set); \
    VM_PROP_PIN_TRAITS_TEST_IMPL_SINGLE(ValueType, Name##Map); \
    VM_PROP_PIN_TRAITS_TEST_IMPL_SINGLE(ValueType, Name##Optional); \
    VM_PROP_PIN_TRAITS_TEST_IMPL_SINGLE(ValueType, Name##ArrayOptional); \

#define VM_PROP_PIN_TRAITS_TEST_REGISTER(Name) \
    VM_PROP_PIN_TRAITS_TEST_REGISTER_SINGLE(Name); \
    VM_PROP_PIN_TRAITS_TEST_REGISTER_SINGLE(Name##Array); \
    VM_PROP_PIN_TRAITS_TEST_REGISTER_SINGLE(Name##Set); \
    VM_PROP_PIN_TRAITS_TEST_REGISTER_SINGLE(Name##Map); \
    VM_PROP_PIN_TRAITS_TEST_REGISTER_SINGLE(Name##Optional); \
    VM_PROP_PIN_TRAITS_TEST_REGISTER_SINGLE(Name##ArrayOptional); \

VM_PROP_PIN_TRAITS_TEST_IMPL(bool, MyBoolean);
VM_PROP_PIN_TRAITS_TEST_IMPL(uint8, MyByte);
VM_PROP_PIN_TRAITS_TEST_IMPL(TSubclassOf<UObject>, MyClass);
VM_PROP_PIN_TRAITS_TEST_IMPL(TSoftClassPtr<UObject>, MySoftClass);
VM_PROP_PIN_TRAITS_TEST_IMPL(int32, MyInt);
VM_PROP_PIN_TRAITS_TEST_IMPL(int64, MyInt64);
VM_PROP_PIN_TRAITS_TEST_IMPL(float, MyFloat);
VM_PROP_PIN_TRAITS_TEST_IMPL(double, MyDouble);
VM_PROP_PIN_TRAITS_TEST_IMPL(FName, MyName);
VM_PROP_PIN_TRAITS_TEST_IMPL(UObject*, MyObject);
VM_PROP_PIN_TRAITS_TEST_IMPL(TObjectPtr<UObject>, MyObjectPtr);
VM_PROP_PIN_TRAITS_TEST_IMPL(TScriptInterface<IPinTraitsInterface>, MyInterface);
VM_PROP_PIN_TRAITS_TEST_IMPL(TSoftObjectPtr<UTexture2D>, MySoftObject);
VM_PROP_PIN_TRAITS_TEST_IMPL(FString, MyString);
VM_PROP_PIN_TRAITS_TEST_IMPL(FText, MyText);
VM_PROP_PIN_TRAITS_TEST_IMPL(FPinTraitsStruct, MyStruct);
VM_PROP_PIN_TRAITS_TEST_IMPL(EPinTraitsEnum, MyEnum);
VM_PROP_PIN_TRAITS_TEST_IMPL(EPinTraitsSimpleEnum, MySimpleEnum);
VM_PROP_PIN_TRAITS_TEST_IMPL(FTimespan, MyTimespan);

struct FPinTraitsPropertiesRegistrator
{
    FPinTraitsPropertiesRegistrator()
    {
        VM_PROP_PIN_TRAITS_TEST_REGISTER(MyBoolean);
        VM_PROP_PIN_TRAITS_TEST_REGISTER(MyByte);
        VM_PROP_PIN_TRAITS_TEST_REGISTER(MyClass);
        VM_PROP_PIN_TRAITS_TEST_REGISTER(MySoftClass);
        VM_PROP_PIN_TRAITS_TEST_REGISTER(MyInt);
        VM_PROP_PIN_TRAITS_TEST_REGISTER(MyInt64);
        VM_PROP_PIN_TRAITS_TEST_REGISTER(MyFloat);
        VM_PROP_PIN_TRAITS_TEST_REGISTER(MyDouble);
        VM_PROP_PIN_TRAITS_TEST_REGISTER(MyName);
        VM_PROP_PIN_TRAITS_TEST_REGISTER(MyObject);
        VM_PROP_PIN_TRAITS_TEST_REGISTER(MyObjectPtr);
        VM_PROP_PIN_TRAITS_TEST_REGISTER(MyInterface);
        VM_PROP_PIN_TRAITS_TEST_REGISTER(MySoftObject);
        VM_PROP_PIN_TRAITS_TEST_REGISTER(MyString);
        VM_PROP_PIN_TRAITS_TEST_REGISTER(MyText);
        VM_PROP_PIN_TRAITS_TEST_REGISTER(MyStruct);
        VM_PROP_PIN_TRAITS_TEST_REGISTER(MyEnum);
        VM_PROP_PIN_TRAITS_TEST_REGISTER(MySimpleEnum);
    }
} GPinTraitsPropertiesRegistrator;

#undef VM_PROP_PIN_TRAITS_TEST_IMPL_SINGLE
#undef VM_PROP_PIN_TRAITS_TEST_REGISTER_SINGLE
#undef VM_PROP_PIN_TRAITS_TEST_IMPL
#undef VM_PROP_PIN_TRAITS_TEST_REGISTER
