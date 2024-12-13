// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Misc/AutomationTest.h"

#include "DerivedViewModel.h"
#include "PinTraitsViewModel.h"
#include "TokenStreamTestViewModel.h"
#include "Mvvm/Impl/Property/ViewModelPropertyIterator.h"
#include "Mvvm/Impl/Property/TokenStreamUtils.h"

#include "Components/PanelWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CheckBox.h"
#include "Components/StaticMeshComponent.h"

using namespace UnrealMvvm_Impl;

BEGIN_DEFINE_SPEC(ViewModelRegistrySpec, "UnrealMvvm.ViewModelRegistry", EAutomationTestFlags::ClientContext | EAutomationTestFlags::EditorContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
TArray<FField*> GetFields(UClass* Class);
UClass* MakeTempClass(UClass* Class);
END_DEFINE_SPEC(ViewModelRegistrySpec)

void ViewModelRegistrySpec::Define()
{
    Describe("GetProperties", [this]()
    {
        It("Should Return Properties Of Base And Derived ViewModels", [this]()
        {
            TArray<const FViewModelPropertyReflection*> Properties = FViewModelPropertyIterator(UDerivedClassViewModel::StaticClass(), true).ToArray();

            TestEqual("Num properties", Properties.Num(), 2);
            TestEqual("Properties[0]", Properties[0]->GetProperty(), (const FViewModelPropertyBase*)UBaseClassViewModel::BaseClassValueProperty());
            TestEqual("Properties[1]", Properties[1]->GetProperty(), (const FViewModelPropertyBase*)UDerivedClassViewModel::DerivedClassValueProperty());
        });

        It("Should Return Properties Of Base ViewModel", [this]()
        {
            TArray<const FViewModelPropertyReflection*> Properties = FViewModelPropertyIterator(UBaseClassViewModel::StaticClass(), true).ToArray();

            TestEqual("Num properties", Properties.Num(), 1);
            TestEqual("Properties[0]", Properties[0]->GetProperty(), (const FViewModelPropertyBase*)UBaseClassViewModel::BaseClassValueProperty());
        });

        It("Should Return Zero Properties Of nullptr", [this]()
        {
            TArray<const FViewModelPropertyReflection*> Properties = FViewModelPropertyIterator(nullptr, true).ToArray();

            TestEqual("Num properties", Properties.Num(), 0);
        });

        It("Should Not Contain Duplicate Properties", [this]
        {
            for (auto& Pair : FViewModelRegistry::GetAllProperties())
            {
                auto ArrayView = MakeArrayView(Pair.Value);

                for (int32 Index = 0; Index < Pair.Value.Num() - 1; ++Index)
                {
                    auto* FoundPtr = Algo::FindByPredicate(ArrayView.RightChop(Index + 1), [&](const FViewModelPropertyReflection& Reflection)
                    {
                        return Reflection.GetProperty() == Pair.Value[Index].GetProperty();
                    });

                    TestNull(
                        FString::Printf(TEXT("Found duplicate %s.%s"), *Pair.Key->GetName(), *Pair.Value[Index].GetProperty()->GetName().ToString()),
                        FoundPtr
                    );
                }
            }
        });
    });

    Describe("FindProperty", [this]()
    {
        It("Should Find Property Of Base ViewModel", [this]()
        {
            const FViewModelPropertyReflection* Property = FViewModelRegistry::FindProperty<UBaseClassViewModel>(TEXT("BaseClassValue"));

            TestNotNull("Property", Property);
            TestEqual("Property Pointer", Property->GetProperty(), (const FViewModelPropertyBase*)UBaseClassViewModel::BaseClassValueProperty());
        });

        It("Should Find Property Of Derived ViewModel", [this]()
        {
            const FViewModelPropertyReflection* Property = FViewModelRegistry::FindProperty<UDerivedClassViewModel>(TEXT("DerivedClassValue"));

            TestNotNull("Property", Property);
            TestEqual("Property Pointer", Property->GetProperty(), (const FViewModelPropertyBase*)UDerivedClassViewModel::DerivedClassValueProperty());
        });

        It("Should Find Property Of Base ViewModel In Derived ViewModel", [this]()
        {
            const FViewModelPropertyReflection* Property = FViewModelRegistry::FindProperty<UDerivedClassViewModel>(TEXT("BaseClassValue"));

            TestNotNull("Property", Property);
            TestEqual("Property Pointer", Property->GetProperty(), (const FViewModelPropertyBase*)UBaseClassViewModel::BaseClassValueProperty());
        });
    });

    Describe("ReferenceTokenStream", [this]
    {
        It("Should Add Derived Classes To List", [this]
        {
            TArray<UClass*> Classes = { UPanelWidget::StaticClass() };
            FTokenStreamUtils::EnrichWithDerivedClasses(Classes);

            // check for direct descendant
            TestTrue("Contains UCanvasPanel", Classes.Contains(UCanvasPanel::StaticClass()));

            // check for transitive descendant
            TestTrue("Contains UCheckBox", Classes.Contains(UCheckBox::StaticClass()));
        });

        It("Should Sort Classes By Inheritance Hierarchy", [this]
        {
            TArray<UClass*> Classes =
            {
                AActor::StaticClass(), APlayerController::StaticClass(), AController::StaticClass(),
                UActorComponent::StaticClass(), UStaticMeshComponent::StaticClass(), UPrimitiveComponent::StaticClass(), UMeshComponent::StaticClass(), USceneComponent::StaticClass(),
                UTexture2D::StaticClass(), UTexture::StaticClass(), UStreamableRenderAsset::StaticClass(),
            };
            FTokenStreamUtils::SortViewModelClasses(Classes);

#define TestOrder(A, B) \
            TestTrue(#A " before " #B, Classes.Find(A::StaticClass()) < Classes.Find(B::StaticClass()))

            TestOrder(AActor, AController);
            TestOrder(AController, APlayerController);

            TestOrder(UActorComponent, USceneComponent);
            TestOrder(USceneComponent, UPrimitiveComponent);
            TestOrder(UPrimitiveComponent, UMeshComponent);
            TestOrder(UMeshComponent, UStaticMeshComponent);

            TestOrder(UStreamableRenderAsset, UTexture);
            TestOrder(UTexture, UTexture2D);

#undef TestOrder
        });

        It("Should Add Properties To Class Without Own Properties", [this]
        {
            UClass* TargetClass = MakeTempClass(UTokenStreamTargetClass_NoProperties::StaticClass());
            auto Properties = FViewModelRegistry::GetAllProperties()[UTokenStreamTestViewModel::StaticClass()];
            FField* ExpectedFirstField = TargetClass->ChildProperties;

            FField* FirstField = FTokenStreamUtils::AddPropertiesToClass(TargetClass, MakeArrayView(Properties));
            TArray<FField*> Fields = GetFields(TargetClass);

            TestEqual("FirstField", FirstField, ExpectedFirstField);

            TestEqual("Fields.Num()", Fields.Num(), 3);
            TestEqual("Fields[0]", Fields[0]->GetName(), TEXT("Pointer"));
            TestEqual("Fields[1]", Fields[1]->GetName(), TEXT("PointerMap"));
            TestEqual("Fields[2]", Fields[2]->GetName(), TEXT("PointerSet"));

            // perform cleanup
            TArray<FField*> KeptProperties;
            FTokenStreamUtils::CleanupProperties(TargetClass, FirstField, KeptProperties);
            for (FField* Field : KeptProperties)
            {
                delete Field;
            }
        });

        It("Should Add Properties To Class With Own Properties", [this]
        {
            UClass* TargetClass = MakeTempClass(UTokenStreamTargetClass_WithProperties::StaticClass());
            auto Properties = FViewModelRegistry::GetAllProperties()[UTokenStreamTestViewModel::StaticClass()];
            FField* ExpectedFirstField = TargetClass->ChildProperties;

            FField* FirstField = FTokenStreamUtils::AddPropertiesToClass(TargetClass, MakeArrayView(Properties));
            TArray<FField*> Fields = GetFields(TargetClass);

            TestEqual("FirstField", FirstField, ExpectedFirstField);

            TestEqual("Fields.Num()", Fields.Num(), 4);
            TestEqual("Fields[0]", Fields[0]->GetName(), TEXT("Pointer"));
            TestEqual("Fields[1]", Fields[1]->GetName(), TEXT("PointerMap"));
            TestEqual("Fields[2]", Fields[2]->GetName(), TEXT("PointerSet"));
            TestEqual("Fields[3]", Fields[3]->GetName(), TEXT("FirstProperty"));

            // perform cleanup
            TArray<FField*> KeptProperties;
            FTokenStreamUtils::CleanupProperties(TargetClass, FirstField, KeptProperties);
            for (FField* Field : KeptProperties)
            {
                delete Field;
            }
        });

        It("Should Remove Properties From Class Without Own Properties", [this]
        {
            UClass* TargetClass = MakeTempClass(UTokenStreamTargetClass_NoProperties::StaticClass());
            auto Properties = FViewModelRegistry::GetAllProperties()[UTokenStreamTestViewModel::StaticClass()];
            FField* ExpectedFirstField = TargetClass->ChildProperties;
            FProperty* ExpectedFirstProperty = TargetClass->PropertyLink;

            FField* FirstField = FTokenStreamUtils::AddPropertiesToClass(TargetClass, MakeArrayView(Properties));

            TArray<FField*> KeptProperties;
            FTokenStreamUtils::CleanupProperties(TargetClass, FirstField, KeptProperties);

            TArray<FField*> Fields = GetFields(TargetClass);

            TestEqual("TargetClass->ChildProperties", TargetClass->ChildProperties, ExpectedFirstField);
            TestEqual("TargetClass->PropertyLink", TargetClass->PropertyLink, ExpectedFirstProperty);

            TestEqual("Fields.Num()", Fields.Num(), 0);

            TestEqual("KeptProperties.Num()", KeptProperties.Num(), 2);
            TestEqual("KeptProperties[0]", KeptProperties[0]->GetName(), TEXT("PointerMap"));
            TestEqual("KeptProperties[1]", KeptProperties[1]->GetName(), TEXT("PointerSet"));

            // perform cleanup
            for (FField* Field : KeptProperties)
            {
                delete Field;
            }
        });

        It("Should Remove Properties From Class With Own Properties", [this]
        {
            UClass* TargetClass = MakeTempClass(UTokenStreamTargetClass_WithProperties::StaticClass());
            auto Properties = FViewModelRegistry::GetAllProperties()[UTokenStreamTestViewModel::StaticClass()];
            FField* ExpectedFirstField = TargetClass->ChildProperties;
            FProperty* ExpectedFirstProperty = TargetClass->PropertyLink;

            FField* FirstField = FTokenStreamUtils::AddPropertiesToClass(TargetClass, MakeArrayView(Properties));

            TArray<FField*> KeptProperties;
            FTokenStreamUtils::CleanupProperties(TargetClass, FirstField, KeptProperties);

            TArray<FField*> Fields = GetFields(TargetClass);

            TestEqual("TargetClass->ChildProperties", TargetClass->ChildProperties, ExpectedFirstField);
            TestEqual("TargetClass->PropertyLink", TargetClass->PropertyLink, ExpectedFirstProperty);

            TestEqual("Fields.Num()", Fields.Num(), 1);
            TestEqual("Fields[0]", Fields[0]->GetName(), TEXT("FirstProperty"));

            TestEqual("KeptProperties.Num()", KeptProperties.Num(), 2);
            TestEqual("KeptProperties[0]", KeptProperties[0]->GetName(), TEXT("PointerMap"));
            TestEqual("KeptProperties[1]", KeptProperties[1]->GetName(), TEXT("PointerSet"));

            // perform cleanup
            for (FField* Field : KeptProperties)
            {
                delete Field;
            }
        });
    });

    Describe("GetValue Operation", [this]()
    {
        Describe("Regular", [this]()
        {
            It("Should Get int32", [this]()
            {
                UPinTraitsViewModel* ViewModel = NewObject<UPinTraitsViewModel>();
                const FViewModelPropertyReflection* Property = FViewModelRegistry::FindProperty<UPinTraitsViewModel>(TEXT("MyInt"));

                int32 OutValue = 0;
                bool HasValue = false;

                ViewModel->SetMyInt(123);
                Property->GetOperations().GetValue(ViewModel, &OutValue, HasValue);

                TestEqual("OutValue", OutValue, 123);
                TestTrue("HasValue", HasValue);
            });

            It("Should Get UObject*", [this]()
            {
                UPinTraitsViewModel* ViewModel = NewObject<UPinTraitsViewModel>();
                const FViewModelPropertyReflection* Property = FViewModelRegistry::FindProperty<UPinTraitsViewModel>(TEXT("MyObject"));

                UObject* OutValue = 0;
                bool HasValue = false;

                ViewModel->SetMyObject(ViewModel);
                Property->GetOperations().GetValue(ViewModel, &OutValue, HasValue);

                TestEqual("OutValue", OutValue, (UObject*)ViewModel);
                TestTrue("HasValue", HasValue);
            });

            It("Should Get TArray", [this]()
            {
                UPinTraitsViewModel* ViewModel = NewObject<UPinTraitsViewModel>();
                const FViewModelPropertyReflection* Property = FViewModelRegistry::FindProperty<UPinTraitsViewModel>(TEXT("MyIntArray"));

                TArray<int32> OutValue;
                bool HasValue = false;

                ViewModel->SetMyIntArray({ 123, 321 });
                Property->GetOperations().GetValue(ViewModel, &OutValue, HasValue);

                TestEqual("OutValue.Num", OutValue.Num(), 2);
                TestEqual("OutValue[0]", OutValue[0], 123);
                TestEqual("OutValue[1]", OutValue[1], 321);
                TestTrue("HasValue", HasValue);
            });
        });

        Describe("Optional", [this]()
        {
            It("Should Get Optional int32", [this]()
            {
                UPinTraitsViewModel* ViewModel = NewObject<UPinTraitsViewModel>();
                const FViewModelPropertyReflection* Property = FViewModelRegistry::FindProperty<UPinTraitsViewModel>(TEXT("MyIntOptional"));

                int32 OutValue = 0;
                bool HasValue = false;

                Property->GetOperations().GetValue(ViewModel, &OutValue, HasValue);
                TestFalse("HasValue", HasValue);

                ViewModel->SetMyIntOptional(123);
                Property->GetOperations().GetValue(ViewModel, &OutValue, HasValue);

                TestEqual("OutValue", OutValue, 123);
                TestTrue("HasValue", HasValue);
            });

            It("Should Get Optional UObject*", [this]()
            {
                UPinTraitsViewModel* ViewModel = NewObject<UPinTraitsViewModel>();
                const FViewModelPropertyReflection* Property = FViewModelRegistry::FindProperty<UPinTraitsViewModel>(TEXT("MyObjectOptional"));

                UObject* OutValue = 0;
                bool HasValue = false;

                Property->GetOperations().GetValue(ViewModel, &OutValue, HasValue);
                TestFalse("HasValue", HasValue);

                ViewModel->SetMyObjectOptional(ViewModel);
                Property->GetOperations().GetValue(ViewModel, &OutValue, HasValue);

                TestEqual("OutValue", OutValue, (UObject*)ViewModel);
                TestTrue("HasValue", HasValue);
            });

            It("Should Get Optional TArray", [this]()
            {
                UPinTraitsViewModel* ViewModel = NewObject<UPinTraitsViewModel>();
                const FViewModelPropertyReflection* Property = FViewModelRegistry::FindProperty<UPinTraitsViewModel>(TEXT("MyIntArrayOptional"));

                TArray<int32> OutValue;
                bool HasValue = false;

                Property->GetOperations().GetValue(ViewModel, &OutValue, HasValue);
                TestFalse("HasValue", HasValue);

                ViewModel->SetMyIntArrayOptional(TArray<int32>({ 123, 321 }));
                Property->GetOperations().GetValue(ViewModel, &OutValue, HasValue);

                TestEqual("OutValue.Num", OutValue.Num(), 2);
                TestEqual("OutValue[0]", OutValue[0], 123);
                TestEqual("OutValue[1]", OutValue[1], 321);
                TestTrue("HasValue", HasValue);
            });
        });
    });

    Describe("SetValue Operation", [this]()
    {
        Describe("Regular", [this]()
        {
            It("Should Set int32", [this]()
            {
                UPinTraitsViewModel* ViewModel = NewObject<UPinTraitsViewModel>();
                const FViewModelPropertyReflection* Property = FViewModelRegistry::FindProperty<UPinTraitsViewModel>(TEXT("MyInt"));

                int32 InValue = 123;
                bool HasValue = false; // we intentionally pass false here, because nonoptional property must ignore it

                Property->GetOperations().SetValue(ViewModel, &InValue, HasValue);
                
                TestEqual("Value", ViewModel->GetMyInt(), InValue);
            });

            It("Should Set UObject*", [this]()
            {
                UPinTraitsViewModel* ViewModel = NewObject<UPinTraitsViewModel>();
                const FViewModelPropertyReflection* Property = FViewModelRegistry::FindProperty<UPinTraitsViewModel>(TEXT("MyObject"));

                UObject* InValue = ViewModel;
                bool HasValue = false; // we intentionally pass false here, because nonoptional property must ignore it

                Property->GetOperations().SetValue(ViewModel, &InValue, HasValue);

                TestEqual("Value", ViewModel->GetMyObject(), InValue);
            });

            It("Should Set TArray", [this]()
            {
                UPinTraitsViewModel* ViewModel = NewObject<UPinTraitsViewModel>();
                const FViewModelPropertyReflection* Property = FViewModelRegistry::FindProperty<UPinTraitsViewModel>(TEXT("MyIntArray"));

                TArray<int32> InValue{ 123, 321 };
                bool HasValue = false; // we intentionally pass false here, because nonoptional property must ignore it

                Property->GetOperations().SetValue(ViewModel, &InValue, HasValue);
                TArray<int32> OutValue = ViewModel->GetMyIntArray();

                TestEqual("OutValue.Num", OutValue.Num(), InValue.Num());
                TestEqual("OutValue[0]", OutValue[0], InValue[0]);
                TestEqual("OutValue[1]", OutValue[1], InValue[1]);
            });
        });

        Describe("Optional", [this]()
        {
            It("Should Set Optional int32", [this]()
            {
                UPinTraitsViewModel* ViewModel = NewObject<UPinTraitsViewModel>();
                const FViewModelPropertyReflection* Property = FViewModelRegistry::FindProperty<UPinTraitsViewModel>(TEXT("MyIntOptional"));

                int32 InValue = 123;

                Property->GetOperations().SetValue(ViewModel, &InValue, true);
                TestEqual("Value", ViewModel->GetMyIntOptional(), TOptional<int32>(123));

                Property->GetOperations().SetValue(ViewModel, &InValue, false);
                TestEqual("Value", ViewModel->GetMyIntOptional(), TOptional<int32>());
            });

            It("Should Set Optional UObject*", [this]()
            {
                UPinTraitsViewModel* ViewModel = NewObject<UPinTraitsViewModel>();
                const FViewModelPropertyReflection* Property = FViewModelRegistry::FindProperty<UPinTraitsViewModel>(TEXT("MyObjectOptional"));

                UObject* InValue = ViewModel;

                Property->GetOperations().SetValue(ViewModel, &InValue, true);
                TestEqual("Value", ViewModel->GetMyObjectOptional(), TOptional<UObject*>(ViewModel));

                Property->GetOperations().SetValue(ViewModel, &InValue, false);
                TestEqual("Value", ViewModel->GetMyObjectOptional(), TOptional<UObject*>());
            });

            It("Should Set Optional TArray", [this]()
            {
                UPinTraitsViewModel* ViewModel = NewObject<UPinTraitsViewModel>();
                const FViewModelPropertyReflection* Property = FViewModelRegistry::FindProperty<UPinTraitsViewModel>(TEXT("MyIntArrayOptional"));

                TArray<int32> InValue{ 123, 321 };

                Property->GetOperations().SetValue(ViewModel, &InValue, true);
                TOptional<TArray<int32>> OutValue = ViewModel->GetMyIntArrayOptional();

                TestTrue("OutValue.IsSet", OutValue.IsSet());
                TestEqual("OutValue.Num", OutValue->Num(), InValue.Num());
                TestEqual("OutValue[0]", OutValue.GetValue()[0], InValue[0]);
                TestEqual("OutValue[1]", OutValue.GetValue()[1], InValue[1]);

                Property->GetOperations().SetValue(ViewModel, &InValue, false);
                OutValue = ViewModel->GetMyIntArrayOptional();

                TestFalse("OutValue.IsSet", OutValue.IsSet());
            });
        });
    });
}

TArray<FField*> ViewModelRegistrySpec::GetFields(UClass* Class)
{
    TArray<FField*> Result;

    for (TFieldIterator<FField> It(Class); It; ++It)
    {
        Result.Add(*It);
    }

    return Result;
}

UClass* ViewModelRegistrySpec::MakeTempClass(UClass* Class)
{
    FObjectDuplicationParameters Params(Class, GetTransientPackage());
    Params.bSkipPostLoad = true; // prevent Fatal error during duplication

    UClass* Result = CastChecked<UClass>(StaticDuplicateObjectEx(Params));
    Result->ClassConstructor = Class->ClassConstructor;
    Result->ClassVTableHelperCtorCaller = Class->ClassVTableHelperCtorCaller;
    Result->CppClassStaticFunctions = Class->CppClassStaticFunctions;
    Result->ClassCastFlags = Class->ClassCastFlags;
    Result->ClassWithin = Class->ClassWithin;
    Result->StaticLink(true);

    // make sure it won't be picked up by any TObjectIterator
    Result->MarkAsGarbage();
    Result->GetDefaultObject()->MarkAsGarbage();

    return Result;
}
