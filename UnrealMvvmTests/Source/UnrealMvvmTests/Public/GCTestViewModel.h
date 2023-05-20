// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/BaseViewModel.h"
#include "GCTestInterface.h" // UHT complains if it is not in separate header >:|
#include "GCTestViewModel.generated.h"

UCLASS()
class UGCTestObject : public UObject, public IGCTestInterface
{
    GENERATED_BODY()
};

USTRUCT()
struct FGCTestStruct
{
    GENERATED_BODY()

public:
    UPROPERTY()
    UGCTestObject* ObjPointer;

    UPROPERTY()
    TArray<UGCTestObject*> ObjPointerArray;

    friend uint32 GetTypeHash(const FGCTestStruct& Struct)
    {
        return GetTypeHash(Struct.ObjPointer);
    }

    friend bool operator==(const FGCTestStruct& A, const FGCTestStruct& B)
    {
        return A.ObjPointer == B.ObjPointer;
    }
};

UCLASS()
class UGCTestViewModel : public UBaseViewModel
{
    GENERATED_BODY()

    VM_PROP_AG_AS(UGCTestObject*, Pointer, public, public);
    VM_PROP_AG_AS(TArray<UGCTestObject*>, PointerArray, public, public);
    VM_PROP_AG_AS(const TArray<UGCTestObject*>&, PointerArrayRef, public, public);
    VM_PROP_AG_AS((TMap<UGCTestObject*, int32>), PointerMapKey, public, public);
    VM_PROP_AG_AS((const TMap<UGCTestObject*, int32>&), PointerMapKeyRef, public, public);
    VM_PROP_AG_AS((TMap<int32, UGCTestObject*>), PointerMapValue, public, public);
    VM_PROP_AG_AS((const TMap<int32, UGCTestObject*>&), PointerMapValueRef, public, public);
    VM_PROP_AG_AS((TMap<UGCTestObject*, UGCTestObject*>), PointerMapKeyValue, public, public);
    VM_PROP_AG_AS((const TMap<UGCTestObject*, UGCTestObject*>&), PointerMapKeyValueRef, public, public);

#if ENGINE_MAJOR_VERSION >= 5
    VM_PROP_AG_AS(TObjectPtr<UGCTestObject>, ObjectPtr, public, public);
    VM_PROP_AG_AS(TArray<TObjectPtr<UGCTestObject>>, ObjectPtrArray, public, public);
    VM_PROP_AG_AS(TSet<TObjectPtr<UGCTestObject>>, ObjectPtrSet, public, public);
    VM_PROP_AG_AS((TMap<TObjectPtr<UGCTestObject>, int32>), ObjectPtrMapKey, public, public);
#endif

    VM_PROP_AG_AS(FGCTestStruct, Struct, public, public);
    VM_PROP_AG_AS(TArray<FGCTestStruct>, StructArray, public, public);
    VM_PROP_AG_AS((TMap<FGCTestStruct, int32>), StructMapKey, public, public);
    VM_PROP_AG_AS((TMap<int32, FGCTestStruct>), StructMapValue, public, public);

    VM_PROP_AG_AS(TScriptInterface<IGCTestInterface>, Interface, public, public);
    VM_PROP_AG_AS(TArray<TScriptInterface<IGCTestInterface>>, InterfaceArray, public, public);

    VM_PROP_AG_AS(TSet<UObject*>, PointerSet, public, public);
    VM_PROP_AG_AS(TSet<FGCTestStruct>, StructSet, public, public);

public:
    UGCTestObject* PlainField;
};

UCLASS()
class UGCTestDerivedViewModel : public UGCTestViewModel
{
    GENERATED_BODY()

    VM_PROP_AG_AS(UGCTestObject*, DerivedPointer, public, public);
};