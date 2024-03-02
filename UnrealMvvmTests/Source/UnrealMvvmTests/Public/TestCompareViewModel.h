// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/BaseViewModel.h"
#include "TestCompareViewModel.generated.h"

/* Struct that can be compared using operator== */
struct FTestComparableStruct
{
    int32 Value = 0;

    bool operator==(const FTestComparableStruct& InOther) const
    {
        return Value == InOther.Value;
    }
};

/* Struct that cannot be compared using operator== */
struct FTestNonComparableStruct
{
    int32 Value = 0;
};

/* Struct that can be compared using operator==, but explicitly disabled comparison */
struct FTestComparableDisabledStruct
{
    int32 Value = 0;

    bool operator==(const FTestComparableDisabledStruct& InOther) const
    {
        return Value == InOther.Value;
    }
};

template<>
struct TViewModelPropertyTypeTraits<FTestComparableDisabledStruct> : public TViewModelPropertyTypeTraitsBase<FTestComparableDisabledStruct>
{
    enum { WithSetterComparison = false };
};

/* Struct that can only be compared using Identical method */
struct FTestComparableWithIdenticalStruct
{
    int32 Value = 0;

    bool Identical(const FTestComparableWithIdenticalStruct* Other, uint32 PortFlags) const
    {
        return Value == Other->Value;
    }
};

template<>
struct TStructOpsTypeTraits<FTestComparableWithIdenticalStruct> : public TStructOpsTypeTraitsBase2<FTestComparableWithIdenticalStruct>
{
    enum { WithIdentical = true };
};

UCLASS()
class UTestCompareViewModel : public UBaseViewModel
{
    GENERATED_BODY()

    VM_PROP_AG_AS(int32, IntValue, public) = 0;
    VM_PROP_AG_AS(UObject*, ObjectValue, public) = nullptr;
    VM_PROP_AG_AS(FTestComparableStruct, ComparableStructValue, public);
    VM_PROP_AG_AS(FTestNonComparableStruct, NonComparableStructValue, public);
    VM_PROP_AG_AS(FTestComparableDisabledStruct, ComparableDisabledStructValue, public);
    VM_PROP_AG_AS(FTestComparableWithIdenticalStruct, ComparableWithIdenticalStructValue, public);

    VM_PROP_AG_AS(TArray<FTestComparableStruct>, ComparableStructArrayValue, public);
    VM_PROP_AG_AS(TArray<FTestNonComparableStruct>, NonComparableStructArrayValue, public);

    VM_PROP_AG_AS(TOptional<FTestComparableStruct>, ComparableStructOptionalValue, public);
    VM_PROP_AG_AS(TOptional<FTestNonComparableStruct>, NonComparableStructOptionalValue, public);

    VM_PROP_AG_AS(TOptional<TOptional<FTestComparableStruct>>, ComparableStructOptionalOptionalValue, public);
    VM_PROP_AG_AS(TOptional<TOptional<FTestNonComparableStruct>>, NonComparableStructOptionalOptionalValue, public);
};
