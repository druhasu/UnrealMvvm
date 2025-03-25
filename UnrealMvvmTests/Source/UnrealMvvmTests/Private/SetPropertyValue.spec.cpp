// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Misc/AutomationTest.h"

#include "Mvvm/MvvmBlueprintLibrary.h"
#include "Mvvm/Impl/Property/ViewModelRegistry.h"
#include "Mvvm/Impl/Property/ViewModelPropertyReflection.h"
#include "PinTraitsViewModel.h"
#include "Blueprint/UserWidget.h"
#include "TempWorldHelper.h"

BEGIN_DEFINE_SPEC(FSetPropertyValueSpec, "UnrealMvvm.SetPropertyValue", EAutomationTestFlags::ClientContext | EAutomationTestFlags::EditorContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
UClass* LoadClass() const;
bool IsTestFunction(UFunction* Function) const;
void TestCommon(FName FunctionName);
END_DEFINE_SPEC(FSetPropertyValueSpec)

void FSetPropertyValueSpec::Define()
{
    UClass* ViewClass = LoadClass();
    if (!ensure(ViewClass != nullptr))
    {
        return;
    }

    ForEachObjectWithOuter(ViewClass, [&](UObject* Obj)
    {
        if (UFunction* Function = Cast<UFunction>(Obj); Function != nullptr && IsTestFunction(Function))
        {
            It(FString::Printf(TEXT("Should %s"), *Function->GetName()), [this, FunctionName = Function->GetFName()]()
            {
                TestCommon(FunctionName);
            });
        }
    });
}

UClass* FSetPropertyValueSpec::LoadClass() const
{
    return StaticLoadClass(UUserWidget::StaticClass(), nullptr, TEXT("/UnrealMvvmTests/WidgetView/BP_TestWidgetView_SetProperty.BP_TestWidgetView_SetProperty_C"));
}

bool FSetPropertyValueSpec::IsTestFunction(UFunction* Function) const
{
    if (Function->NumParms != 2)
    {
        return false;
    }

    TArray<FProperty*, TFixedAllocator<2>> ParamsProperties;
    for (TFieldIterator<FProperty> It(Function); It; ++It)
    {
        if (It->HasAllPropertyFlags(CPF_Parm | CPF_OutParm))
        {
            ParamsProperties.Add(*It);
        }
    }

    if (ParamsProperties.Num() != 2)
    {
        return false;
    }

    if (CastField<FNameProperty>(ParamsProperties[0]) == nullptr)
    {
        return false;
    }

    return true;
}

void FSetPropertyValueSpec::TestCommon(FName FunctionName)
{
    FTempWorldHelper Helper;

    UClass* ViewClass = LoadClass();
    UFunction* Function = ViewClass->FindFunctionByName(FunctionName);

    UUserWidget* View = CreateWidget<UUserWidget>(Helper.World, ViewClass);
    UPinTraitsViewModel* ViewModel = NewObject<UPinTraitsViewModel>();

    UMvvmBlueprintLibrary::SetViewModel(View, ViewModel);

    void* Parms = FMemory_Alloca(Function->ParmsSize);

    TArray<FProperty*, TFixedAllocator<2>> ParamsProperties;
    for (TFieldIterator<FProperty> It(Function); It; ++It)
    {
        if (It->HasAllPropertyFlags(CPF_Parm | CPF_OutParm))
        {
            ParamsProperties.Add(*It);
            It->InitializeValue((uint8*)Parms + It->GetOffset_ForUFunction());
        }
    }

    View->ProcessEvent(Function, Parms);

    const UnrealMvvm_Impl::FViewModelPropertyReflection* Reflection = UnrealMvvm_Impl::FViewModelRegistry::FindProperty<UPinTraitsViewModel>(*(FName*)Parms);
    TestNotNull("ViewModel property", Reflection);

    void* ActualValue = FMemory_Alloca(Reflection->SizeOfValue);
    bool bHasValue = false;
    ParamsProperties[1]->InitializeValue(ActualValue);
    Reflection->GetOperations().GetValue(ViewModel, ActualValue, bHasValue);

    void* ExpectedValue = (uint8*)Parms + ParamsProperties[1]->GetOffset_ForUFunction();
    TestTrue("Actual value equals to expected", ParamsProperties[1]->Identical(ActualValue, ExpectedValue));

    ParamsProperties[0]->DestroyValue(Parms);
    ParamsProperties[1]->DestroyValue(ActualValue);
    ParamsProperties[1]->DestroyValue(ExpectedValue);
}
