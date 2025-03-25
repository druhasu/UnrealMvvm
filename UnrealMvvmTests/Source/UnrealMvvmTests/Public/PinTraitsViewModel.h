// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Engine/Texture2D.h"
#include "Templates/SubclassOf.h"
#include "UObject/Interface.h"
#include "Mvvm/BaseViewModel.h"
#include "PinTraitsViewModel.generated.h"

UINTERFACE(BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class UNREALMVVMTESTS_API UPinTraitsInterface : public UInterface { GENERATED_BODY() };

class UNREALMVVMTESTS_API IPinTraitsInterface
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

    friend uint32 GetTypeHash(const FPinTraitsStruct& S) { return GetTypeHash(S.Name); }
};

UENUM(BlueprintType)
enum class EPinTraitsEnum : uint8
{
    First, Second, Third
};

enum class EPinTraitsSimpleEnum : uint8
{
    First, Second, Third
};

#define VM_PROP_PIN_TRAITS_TEST(ValueType, Name) \
    VM_PROP_AG_AS(ValueType, Name, public, public); \
    VM_PROP_AG_AS(TArray<ValueType>, Name##Array, public, public); \
    VM_PROP_AG_AS(TSet<ValueType>, Name##Set, public, public); \
    VM_PROP_AG_AS((TMap<FName, ValueType>), Name##Map, public, public); \
    VM_PROP_AG_AS(TOptional<ValueType>, Name##Optional, public, public); \
    VM_PROP_AG_AS(TOptional<TArray<ValueType>>, Name##ArrayOptional, public, public);

UCLASS()
class UNREALMVVMTESTS_API UPinTraitsViewModel : public UBaseViewModel
{
    GENERATED_BODY()

    VM_PROP_PIN_TRAITS_TEST(bool, MyBoolean);
    VM_PROP_PIN_TRAITS_TEST(uint8, MyByte);
    VM_PROP_PIN_TRAITS_TEST(TSubclassOf<UObject>, MyClass);
    VM_PROP_PIN_TRAITS_TEST(TSoftClassPtr<UObject>, MySoftClass);
    VM_PROP_PIN_TRAITS_TEST(int32, MyInt);
    VM_PROP_PIN_TRAITS_TEST(int64, MyInt64);
    VM_PROP_PIN_TRAITS_TEST(float, MyFloat);
    VM_PROP_PIN_TRAITS_TEST(double, MyDouble);
    VM_PROP_PIN_TRAITS_TEST(FName, MyName);
    VM_PROP_PIN_TRAITS_TEST(UObject*, MyObject);
    VM_PROP_PIN_TRAITS_TEST(TObjectPtr<UObject>, MyObjectPtr);
    VM_PROP_PIN_TRAITS_TEST(TScriptInterface<IPinTraitsInterface>, MyInterface);
    VM_PROP_PIN_TRAITS_TEST(TSoftObjectPtr<UTexture2D>, MySoftObject);
    VM_PROP_PIN_TRAITS_TEST(FString, MyString);
    VM_PROP_PIN_TRAITS_TEST(FText, MyText);
    VM_PROP_PIN_TRAITS_TEST(FPinTraitsStruct, MyStruct);
    VM_PROP_PIN_TRAITS_TEST(EPinTraitsEnum, MyEnum);
    VM_PROP_PIN_TRAITS_TEST(EPinTraitsSimpleEnum, MySimpleEnum);
    VM_PROP_PIN_TRAITS_TEST(FTimespan, MyTimespan);

    VM_PROP_AG_AS(TSharedPtr<EPinTraitsEnum>, Unsupported, public, public);
};

UCLASS(BlueprintType)
class UNREALMVVMTESTS_API UPinTraitsInterfaceImpl : public UObject, public IPinTraitsInterface
{
    GENERATED_BODY()

public:
    FString GetSomeString() override { return GetName(); }
};
