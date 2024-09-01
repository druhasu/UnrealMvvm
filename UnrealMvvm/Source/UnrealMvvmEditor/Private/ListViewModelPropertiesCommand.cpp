// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Misc/CoreMisc.h"
#include "Mvvm/BaseViewModel.h"
#include "Mvvm/Impl/Property/ViewModelRegistry.h"
#include "Nodes/ViewModelPropertyNodeHelper.h"

class FListViewModelPropertiesCommand : public FSelfRegisteringExec
{
public:
    bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override;

private:
    struct FPropertyLogInfo
    {
        FString Type;
        FString Name;
        FString Attributes;
    };

    struct FViewModelLogInfo
    {
        FString Name;
        TArray<FPropertyLogInfo> Properties;
    };

    static FString GetValueTypeName(const UnrealMvvm_Impl::FViewModelPropertyReflection& Reflection);
    static FString GetValueElementTypeName(UnrealMvvm_Impl::EPinCategoryType PinCategoryType, UObject* SubCategoryObject);
    static FString GetAttributes(const UnrealMvvm_Impl::FViewModelPropertyReflection& Reflection);
}
GListViewModelPropertiesCommand;

bool FListViewModelPropertiesCommand::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
    using namespace UnrealMvvm_Impl;

    if (FParse::Command(&Cmd, TEXT("ListViewModelProperties")))
    {
        UE_LOG(LogTemp, Log, TEXT("Listing all registered ViewModel classes and their properties"));

        TArray<FViewModelLogInfo> ViewModels;

        // Iterate over all properties and collect data
        int32 MaxTypeLength = 0;
        int32 MaxNameLength = 0;
        for (const TPair<UClass*, TArray<FViewModelPropertyReflection>>& Pair : FViewModelRegistry::GetAllProperties())
        {
            FViewModelLogInfo& ViewModelInfo = ViewModels.Emplace_GetRef();
            ViewModelInfo.Name = Pair.Key->GetName();

            for (const FViewModelPropertyReflection& Reflection : Pair.Value)
            {
                FPropertyLogInfo& PropertyInfo = ViewModelInfo.Properties.Emplace_GetRef();
                PropertyInfo.Type = GetValueTypeName(Reflection);
                PropertyInfo.Name = Reflection.GetProperty()->GetName().ToString();
                PropertyInfo.Attributes = GetAttributes(Reflection);

                MaxTypeLength = FMath::Max(MaxTypeLength, PropertyInfo.Type.Len());
                MaxNameLength = FMath::Max(MaxNameLength, PropertyInfo.Name.Len());
            }
        }

        // Output collected data with compact alignment
        for (const FViewModelLogInfo& ViewModelInfo : ViewModels)
        {
            UE_LOG(LogTemp, Log, TEXT("  %s:"), *ViewModelInfo.Name);

            for (const FPropertyLogInfo& PropertyInfo : ViewModelInfo.Properties)
            {
                UE_LOG(LogTemp, Log, TEXT("    %-*s  %-*s %s"),
                    MaxTypeLength,
                    *PropertyInfo.Type,
                    MaxNameLength,
                    *PropertyInfo.Name,
                    *PropertyInfo.Attributes);

            }

            UE_LOG(LogTemp, Log, TEXT("  "));
        }

        return true;
    }

    return false;
}

FString FListViewModelPropertiesCommand::GetValueTypeName(const UnrealMvvm_Impl::FViewModelPropertyReflection& Reflection)
{
    using namespace UnrealMvvm_Impl;

    FString ValueTypeName = TEXT("<unknown>");

    if (Reflection.PinCategoryType != EPinCategoryType::Unsupported)
    {
        FString ValueElementName = GetValueElementTypeName(Reflection.PinCategoryType, Reflection.GetPinSubCategoryObject());

        switch (Reflection.ContainerType)
        {
            case EPinContainerType::None:
                ValueTypeName = ValueElementName;
                break;

            case EPinContainerType::Array:
                ValueTypeName = FString::Printf(TEXT("TArray<%s>"), *ValueElementName);
                break;

            case EPinContainerType::Set:
                ValueTypeName = FString::Printf(TEXT("TSet<%s>"), *ValueElementName);
                break;

            case EPinContainerType::Map:
                FString MapValueType = GetValueElementTypeName(Reflection.PinValueCategoryType, Reflection.GetPinValueSubCategoryObject());
                ValueTypeName = FString::Printf(TEXT("TMap<%s, %s>"), *ValueElementName, *MapValueType);
                break;
        }

        if (Reflection.Flags.IsOptional)
        {
            ValueTypeName = FString::Printf(TEXT("TOptional<%s>"), *ValueTypeName);
        }
    }

    return ValueTypeName;
}

FString FListViewModelPropertiesCommand::GetValueElementTypeName(UnrealMvvm_Impl::EPinCategoryType PinCategoryType, UObject* SubCategoryObject)
{
    using namespace UnrealMvvm_Impl;

    switch (PinCategoryType)
    {
        case EPinCategoryType::Class:
        case EPinCategoryType::Object:
        case EPinCategoryType::Struct:
            return CastChecked<UStruct>(SubCategoryObject)->GetPrefixCPP() + SubCategoryObject->GetName();

        case EPinCategoryType::SoftClass:
            return FString::Printf(TEXT("TSoftClassPtr<U%s>"), *SubCategoryObject->GetName());

        case EPinCategoryType::Interface:
            return FString::Printf(TEXT("TScriptInterface<I%s>"), *SubCategoryObject->GetName());

        case EPinCategoryType::SoftObject:
            return FString::Printf(TEXT("TSoftObjectPtr<U%s>"), *SubCategoryObject->GetName());

        case EPinCategoryType::Enum:
            return SubCategoryObject ? SubCategoryObject->GetName() : TEXT("_NativeEnum_");

        case EPinCategoryType::Boolean: return TEXT("bool");
        case EPinCategoryType::Byte:    return TEXT("uint8");
        case EPinCategoryType::Int:     return TEXT("int32");
        case EPinCategoryType::Int64:   return TEXT("int64");
        case EPinCategoryType::Float:   return TEXT("float");
#if ENGINE_MAJOR_VERSION >= 5
        case EPinCategoryType::Double:   return TEXT("double");
#endif
        case EPinCategoryType::Name:    return TEXT("FName");
        case EPinCategoryType::String:  return TEXT("FString");
        case EPinCategoryType::Text:    return TEXT("FText");

        default:                        return TEXT("<unknown>");
    }
}

FString FListViewModelPropertiesCommand::GetAttributes(const UnrealMvvm_Impl::FViewModelPropertyReflection& Reflection)
{
    using namespace UnrealMvvm_Impl;

    TArray<FString> Attributes;
    if (Reflection.GetOperations().ContainsObjectReference(false))
    {
        Attributes.Emplace(TEXT("HasReference"));
    }

    if (FViewModelPropertyNodeHelper::IsPropertyAvailableInBlueprint(Reflection) && Reflection.Flags.HasPublicGetter)
    {
        Attributes.Emplace(TEXT("UmgBindable"));
    }

    if (Reflection.Flags.IsOptional)
    {
        Attributes.Emplace(TEXT("Optional"));
    }

    if (Reflection.Flags.HasPublicGetter)
    {
        Attributes.Emplace(TEXT("PublicGetter"));
    }

    if (Reflection.Flags.HasPublicSetter)
    {
        Attributes.Emplace(TEXT("PublicSetter"));
    }

    if (Attributes.Num() > 0)
    {
        return FString::Join(Attributes, TEXT(", "));
    }

    return TEXT("-");
}