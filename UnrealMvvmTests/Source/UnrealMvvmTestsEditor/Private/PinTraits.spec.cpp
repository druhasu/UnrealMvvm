// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Misc/AutomationTest.h"

#include "PinTraitsViewModel.h"
#include "Mvvm/Impl/Property/ViewModelPropertyReflection.h"
#include "Mvvm/Impl/Property/ViewModelRegistry.h"

using namespace UnrealMvvm_Impl;

struct FTestCase
{
    FString BaseName;
    EPinCategoryType CategoryType;
    UObject* SubCategoryObject;
};

BEGIN_DEFINE_SPEC(PinTraitsSpec, "UnrealMvvm.PinTraits", EAutomationTestFlags::ClientContext | EAutomationTestFlags::EditorContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)

void TestSingleValue(const FTestCase& TestCase);
void TestArray(const FTestCase& TestCase);
void TestSet(const FTestCase& TestCase);
void TestMap(const FTestCase& TestCase);
template <typename TType>
void TestBaseStructure(UObject* Struct);
void TestOptional(const FTestCase& TestCase);
TArray<FTestCase> GetTestCases() const;
const TCHAR* ToString(EPinCategoryType Type);

END_DEFINE_SPEC(PinTraitsSpec)

void PinTraitsSpec::Define()
{
    It("Should pass EPinCategoryType::Unsupported for Unsupported type", [this]()
    {
        const FViewModelPropertyReflection* Reflection = FViewModelRegistry::FindProperty(UPinTraitsViewModel::StaticClass(), FName("Unsupported"));

        TestNotNull("ReflectionInfo", Reflection);
        TestEqual("PinType", Reflection->PinCategoryType, EPinCategoryType::Unsupported);
    });

    Describe("Single Value", [this]()
    {
        for (auto& TestCase : GetTestCases())
        {
            FString Desc = FString::Printf(TEXT("Should pass EPinCategoryType::%s for %s"), ToString(TestCase.CategoryType), *TestCase.BaseName);
            It(Desc, [this, TestCase]()
            {
                TestSingleValue(TestCase);
            });
        }
    });

    Describe("Array", [this]()
    {
        for (auto& TestCase : GetTestCases())
        {
            FString Desc = FString::Printf(TEXT("Should pass EPinCategoryType::%s for %sArray"), ToString(TestCase.CategoryType), *TestCase.BaseName);
            It(Desc, [this, TestCase]()
            {
                TestArray(TestCase);
            });
        }
    });

    Describe("Set", [this]()
    {
        for (auto& TestCase : GetTestCases())
        {
            FString Desc = FString::Printf(TEXT("Should pass EPinCategoryType::%s for %sSet"), ToString(TestCase.CategoryType), *TestCase.BaseName);
            It(Desc, [this, TestCase]()
            {
                TestSet(TestCase);
            });
        }
    });

    Describe("Map", [this]()
    {
        for (auto& TestCase : GetTestCases())
        {
            FString Desc = FString::Printf(TEXT("Should pass EPinCategoryType::%s for %sMap"), ToString(TestCase.CategoryType), *TestCase.BaseName);
            It(Desc, [this, TestCase]()
            {
                TestMap(TestCase);
            });
        }
    });

    Describe("Base Structure", [this]()
    {
    #define BASE_STRUCTURE_TEST_CASE(Type) \
        It("Should pass EPinCategory::Struct for " #Type, \
        [this]() { TestBaseStructure<Type>(TBaseStructure<Type>::Get()); })

        // list just several of them
        BASE_STRUCTURE_TEST_CASE(FRotator);
        BASE_STRUCTURE_TEST_CASE(FQuat);
        BASE_STRUCTURE_TEST_CASE(FTransform);
        BASE_STRUCTURE_TEST_CASE(FLinearColor);
        BASE_STRUCTURE_TEST_CASE(FColor);
        BASE_STRUCTURE_TEST_CASE(FPlane);
        BASE_STRUCTURE_TEST_CASE(FVector);
        BASE_STRUCTURE_TEST_CASE(FVector2D);
        BASE_STRUCTURE_TEST_CASE(FVector4);
        BASE_STRUCTURE_TEST_CASE(FRandomStream);
        BASE_STRUCTURE_TEST_CASE(FGuid);
        BASE_STRUCTURE_TEST_CASE(FBox2D);
        BASE_STRUCTURE_TEST_CASE(FFloatRangeBound);
        BASE_STRUCTURE_TEST_CASE(FFloatRange);
        BASE_STRUCTURE_TEST_CASE(FInt32RangeBound);
        BASE_STRUCTURE_TEST_CASE(FInt32Range);
        BASE_STRUCTURE_TEST_CASE(FFloatInterval);
        BASE_STRUCTURE_TEST_CASE(FInt32Interval);
        BASE_STRUCTURE_TEST_CASE(FSoftObjectPath);
        BASE_STRUCTURE_TEST_CASE(FSoftClassPath);
        BASE_STRUCTURE_TEST_CASE(FDateTime);

    #undef BASE_STRUCTURE_TEST_CASE
    });

    Describe("Optional", [this]()
    {
        for (auto& TestCase : GetTestCases())
        {
            FString Desc = FString::Printf(TEXT("Should pass EPinCategoryType::%s for %sOptional"), ToString(TestCase.CategoryType), *TestCase.BaseName);
            It(Desc, [this, TestCase]()
            {
                TestOptional(TestCase);
            });
        }
    });
}

void PinTraitsSpec::TestSingleValue(const FTestCase& TestCase)
{
    const FViewModelPropertyReflection* Reflection = FViewModelRegistry::FindProperty(UPinTraitsViewModel::StaticClass(), FName(TestCase.BaseName));

    TestNotNull("ReflectionInfo", Reflection);
    TestEqual("PinType", Reflection->PinCategoryType, TestCase.CategoryType);
    TestEqual("SubCategoryObject", Reflection->GetPinSubCategoryObject(), TestCase.SubCategoryObject);
    TestEqual("ContainerType", Reflection->ContainerType, EPinContainerType::None);
    TestFalse("IsOptional", Reflection->Flags.IsOptional);
}

void PinTraitsSpec::TestArray(const FTestCase& TestCase)
{
    const FViewModelPropertyReflection* Reflection = FViewModelRegistry::FindProperty(UPinTraitsViewModel::StaticClass(), FName(TestCase.BaseName + TEXT("Array")));

    TestNotNull("ReflectionInfo", Reflection);
    TestEqual("PinType", Reflection->PinCategoryType, TestCase.CategoryType);
    TestEqual("SubCategoryObject", Reflection->GetPinSubCategoryObject(), TestCase.SubCategoryObject);
    TestEqual("ContainerType", Reflection->ContainerType, EPinContainerType::Array);
    TestFalse("IsOptional", Reflection->Flags.IsOptional);
}

void PinTraitsSpec::TestSet(const FTestCase& TestCase)
{
    const FViewModelPropertyReflection* Reflection = FViewModelRegistry::FindProperty(UPinTraitsViewModel::StaticClass(), FName(TestCase.BaseName + TEXT("Set")));

    TestNotNull("ReflectionInfo", Reflection);
    TestEqual("PinType", Reflection->PinCategoryType, TestCase.CategoryType);
    TestEqual("SubCategoryObject", Reflection->GetPinSubCategoryObject(), TestCase.SubCategoryObject);
    TestEqual("ContainerType", Reflection->ContainerType, EPinContainerType::Set);
    TestFalse("IsOptional", Reflection->Flags.IsOptional);
}

void PinTraitsSpec::TestMap(const FTestCase& TestCase)
{
    const FViewModelPropertyReflection* Reflection = FViewModelRegistry::FindProperty(UPinTraitsViewModel::StaticClass(), FName(TestCase.BaseName + TEXT("Map")));

    TestNotNull("ReflectionInfo", Reflection);
    TestEqual("PinType", Reflection->PinValueCategoryType, TestCase.CategoryType);
    TestEqual("SubCategoryObject", Reflection->GetPinValueSubCategoryObject(), TestCase.SubCategoryObject);
    TestEqual("ContainerType", Reflection->ContainerType, EPinContainerType::Map);
    TestFalse("IsOptional", Reflection->Flags.IsOptional);
}

template <typename TType>
void PinTraitsSpec::TestBaseStructure(UObject* Struct)
{
    // separate variable to fix weird XCode DebugGameEditor linkage error
    static constexpr EPinCategoryType PinCategoryType = TPinTraits<TType>::PinCategoryType;
    TestEqual("PinType", PinCategoryType, EPinCategoryType::Struct);
    TestEqual("SubCategoryObject", TPinTraits<TType>::GetSubCategoryObject(), Struct);
}

void PinTraitsSpec::TestOptional(const FTestCase& TestCase)
{
    const FViewModelPropertyReflection* Reflection = FViewModelRegistry::FindProperty(UPinTraitsViewModel::StaticClass(), FName(TestCase.BaseName + TEXT("Optional")));

    TestNotNull("ReflectionInfo", Reflection);
    TestEqual("PinType", Reflection->PinCategoryType, TestCase.CategoryType);
    TestEqual("SubCategoryObject", Reflection->GetPinSubCategoryObject(), TestCase.SubCategoryObject);
    TestEqual("ContainerType", Reflection->ContainerType, EPinContainerType::None);
    TestTrue("IsOptional", Reflection->Flags.IsOptional);
}

TArray<FTestCase> PinTraitsSpec::GetTestCases() const
{
    return
    {
        { TEXT("MyBoolean"), EPinCategoryType::Boolean, nullptr },
        { TEXT("MyByte"), EPinCategoryType::Byte, nullptr },
        { TEXT("MyClass"), EPinCategoryType::Class, UClass::StaticClass() },
        { TEXT("MySoftClass"), EPinCategoryType::SoftClass, UClass::StaticClass() },
        { TEXT("MyInt"), EPinCategoryType::Int, nullptr },
        { TEXT("MyInt64"), EPinCategoryType::Int64, nullptr },
        { TEXT("MyFloat"), EPinCategoryType::Float, nullptr },
#if ENGINE_MAJOR_VERSION >= 5
        { TEXT("MyDouble"), EPinCategoryType::Double, nullptr },
#endif
        { TEXT("MyName"), EPinCategoryType::Name, nullptr },
        { TEXT("MyObject"), EPinCategoryType::Object, UObject::StaticClass() },
#if ENGINE_MAJOR_VERSION >= 5
        { TEXT("MyObjectPtr"), EPinCategoryType::Object, UObject::StaticClass() },
#endif
        { TEXT("MyInterface"), EPinCategoryType::Interface, UPinTraitsInterface::StaticClass() },
        { TEXT("MySoftObject"), EPinCategoryType::SoftObject, UTexture2D::StaticClass() },
        { TEXT("MyString"), EPinCategoryType::String, nullptr },
        { TEXT("MyText"), EPinCategoryType::Text, nullptr },
        { TEXT("MyStruct"), EPinCategoryType::Struct, FPinTraitsStruct::StaticStruct() },
        { TEXT("MyEnum"), EPinCategoryType::Enum, StaticEnum<EPinTraitsEnum>() },
        { TEXT("MySimpleEnum"), EPinCategoryType::Enum, nullptr },
    };
}

const TCHAR* PinTraitsSpec::ToString(EPinCategoryType Type)
{
    #define CASE(V) \
        case EPinCategoryType::V: \
            return TEXT(#V)

    switch (Type)
    {
        CASE(Unsupported);
        CASE(Boolean);
        CASE(Byte);
        CASE(Class);
        CASE(SoftClass);
        CASE(Int);
        CASE(Int64);
        CASE(Float);
#if ENGINE_MAJOR_VERSION >= 5
        CASE(Double);
#endif
        CASE(Name);
        CASE(Object);
        CASE(Interface);
        CASE(SoftObject);
        CASE(String);
        CASE(Text);
        CASE(Struct);
        CASE(Enum);

        default:
            return TEXT("");
    }

    #undef CASE
}
