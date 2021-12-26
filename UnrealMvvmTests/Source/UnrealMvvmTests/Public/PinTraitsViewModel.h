// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "UObject/Interface.h"
#include "Mvvm/BaseViewModel.h"
#include "PinTraitsViewModel.generated.h"

UINTERFACE(BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class UPinTraitsInterface : public UInterface { GENERATED_BODY() };

class IPinTraitsInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    virtual FString GetSomeString() = 0;
};

USTRUCT(Blueprintable)
struct FPinTraitsStruct
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FString Name;
};

UENUM(BlueprintType)
enum class EPinTraitsEnum : uint8
{
    First, Second, Third
};

#define VM_PROP_PIN_TRAITS_TEST(ValueType, Name) \
    VM_PROP_AG_AS(ValueType, Name, public, public); \
    VM_PROP_AG_AS(TArray<ValueType>, Name##Array, public, public); \
    VM_PROP_AG_AS((TMap<FName, ValueType>), Name##Map, public, public);

UCLASS()
class UPinTraitsViewModel : public UBaseViewModel
{
    GENERATED_BODY()

    VM_PROP_PIN_TRAITS_TEST(bool, MyBoolean);
    VM_PROP_PIN_TRAITS_TEST(uint8, MyByte);
    VM_PROP_PIN_TRAITS_TEST(TSubclassOf<UClass>, MyClass);
    VM_PROP_PIN_TRAITS_TEST(TSoftClassPtr<UClass>, MySoftClass);
    VM_PROP_PIN_TRAITS_TEST(int32, MyInt);
    VM_PROP_PIN_TRAITS_TEST(int64, MyInt64);
    VM_PROP_PIN_TRAITS_TEST(float, MyFloat);
    VM_PROP_PIN_TRAITS_TEST(FName, MyName);
    VM_PROP_PIN_TRAITS_TEST(UObject*, MyObject);
    VM_PROP_PIN_TRAITS_TEST(TScriptInterface<IPinTraitsInterface>, MyInterface);
    VM_PROP_PIN_TRAITS_TEST(TSoftObjectPtr<UTexture2D>, MySoftObject);
    VM_PROP_PIN_TRAITS_TEST(FString, MyString);
    VM_PROP_PIN_TRAITS_TEST(FText, MyText);
    VM_PROP_PIN_TRAITS_TEST(FPinTraitsStruct, MyStruct);
    VM_PROP_PIN_TRAITS_TEST(EPinTraitsEnum, MyEnum);

    VM_PROP_AG_AS(TSharedPtr<EPinTraitsEnum>, Unsupported, public, public);
};