// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Misc/AutomationTest.h"

#include "GCTestViewModel.h"
#include "UObject/StrongObjectPtr.h"
#include "Mvvm/Impl/PropertyFactory.h"

BEGIN_DEFINE_SPEC(FViewModelGCSpec, "UnrealMvvm.BaseViewModel.Garbage Collect", EAutomationTestFlags::ClientContext | EAutomationTestFlags::EditorContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)

void TestCommon(TFunctionRef<void(UGCTestViewModel*,UGCTestObject*)> Action, bool bExpectAlive = true);
void TestDerived(TFunctionRef<void(UGCTestDerivedViewModel*, UGCTestObject*)> Action, bool bExpectAlive = true);

template <typename TViewModel, typename TAction>
void TestImpl(TAction Action, bool bExpectAlive);

END_DEFINE_SPEC(FViewModelGCSpec)

void FViewModelGCSpec::Define()
{
    It("Should collect object stored in plain field", [this]
    {
        TestCommon([](UGCTestViewModel* ViewModel, auto Obj)
        {
            ViewModel->PlainField = Obj;
        }, false);
    });

    Describe("UObject*", [this]
    {
        It("Should keep object stored as UObject*", [this]
        {
            TestCommon([](UGCTestViewModel* ViewModel, auto Obj)
            {
                ViewModel->SetPointer(Obj);
            });
        });

        It("Should keep object stored in TArray<UObject*>", [this]
        {
            TestCommon([](UGCTestViewModel* ViewModel, auto Obj)
            {
                ViewModel->SetPointerArray({ Obj });
            });
        });

        It("Should keep object stored in TArray<UObject*>&", [this]
        {
            TestCommon([](UGCTestViewModel* ViewModel, auto Obj)
            {
                ViewModel->SetPointerArrayRef({ Obj });
            });
        });

        It("Should keep object stored in TMap<UObject*, _ >", [this]
        {
            TestCommon([](UGCTestViewModel* ViewModel, auto Obj)
            {
                ViewModel->SetPointerMapKey({ { Obj, 1 } });
            });
        });

        It("Should keep object stored in TMap<UObject*, _ >&", [this]
        {
            TestCommon([](UGCTestViewModel* ViewModel, auto Obj)
            {
                ViewModel->SetPointerMapKeyRef({ { Obj, 1 } });
            });
        });

        It("Should keep object stored in TMap< _ , UObject*>", [this]
        {
            TestCommon([](UGCTestViewModel* ViewModel, auto Obj)
            {
                ViewModel->SetPointerMapValue({ { 1, Obj } });
            });
        });

        It("Should keep object stored in TMap< _ , UObject*>&", [this]
        {
            TestCommon([](UGCTestViewModel* ViewModel, auto Obj)
            {
                ViewModel->SetPointerMapValueRef({ { 1, Obj } });
            });
        });

        It("Should keep object stored in TMap<UObject* , UObject*>", [this]
        {
            TestCommon([](UGCTestViewModel* ViewModel, auto Obj)
            {
                ViewModel->SetPointerMapKeyValue({ { Obj, Obj } });
            });
        });

        It("Should keep object stored in TMap<UObject* , UObject*>&", [this]
        {
            TestCommon([](UGCTestViewModel* ViewModel, auto Obj)
            {
                ViewModel->SetPointerMapKeyValueRef({ { Obj, Obj } });
            });
        });

#if ENGINE_MAJOR_VERSION >= 5
        It("Should keep object stored as TObjectPtr<UObject>", [this]
        {
            TestCommon([](UGCTestViewModel* ViewModel, auto Obj)
            {
                ViewModel->SetObjectPtr(Obj);
            });
        });

        It("Should keep object stored in TArray<TObjectPtr<UObject>>", [this]
        {
            TestCommon([](UGCTestViewModel* ViewModel, auto Obj)
            {
                ViewModel->SetObjectPtrArray({ Obj });
            });
        });

        It("Should keep object stored in TSet<TObjectPtr<UObject>>", [this]
        {
            TestCommon([](UGCTestViewModel* ViewModel, auto Obj)
            {
                ViewModel->SetObjectPtrSet({ Obj });
            });
        });

        It("Should keep object stored in TMap<TObjectPtr<UObject>, _ >", [this]
        {
            TestCommon([](UGCTestViewModel* ViewModel, auto Obj)
            {
                ViewModel->SetObjectPtrMapKey({ { Obj, 1 } });
            });
        });
#endif
    });

    Describe("Struct", [this]
    {
        It("Should keep object stored in UStruct::UObject*", [this]
        {
            TestCommon([](UGCTestViewModel* ViewModel, auto Obj)
            {
                ViewModel->SetStruct({ Obj, {} });
            });
        });

        It("Should keep object stored in UStruct::TArray<UObject*>", [this]
        {
            TestCommon([](UGCTestViewModel* ViewModel, auto Obj)
            {
                ViewModel->SetStruct({ nullptr, { Obj } });
            });
        });

        It("Should keep object stored in TArray< UStruct::UObject* >", [this]
        {
            TestCommon([](UGCTestViewModel* ViewModel, auto Obj)
            {
                ViewModel->SetStructArray({ { Obj, {} } });
            });
        });

        It("Should keep object stored in TArray< UStruct::TArray<UObject*> >", [this]
        {
            TestCommon([](UGCTestViewModel* ViewModel, auto Obj)
            {
                ViewModel->SetStructArray({ { nullptr, { Obj } } });
            });
        });

        It("Should keep object stored in TMap<UStruct, _ >", [this]
        {
            TestCommon([](UGCTestViewModel* ViewModel, auto Obj)
            {
                ViewModel->SetStructMapKey({ { { Obj, {} }, 1 } });
            });
        });

        It("Should keep object stored in TMap< _ , UStruct>", [this]
        {
            TestCommon([](UGCTestViewModel* ViewModel, auto Obj)
            {
                ViewModel->SetStructMapValue({ { 1, { Obj, {} } } });
            });
        });
    });

    Describe("TScriptInterface", [this]
    {
        It("Should keep object stored in TScriptInterface<T>", [this]
        {
            TestCommon([](UGCTestViewModel* ViewModel, auto Obj)
            {
                ViewModel->SetInterface({ Obj });
            });
        });

        It("Should keep object stored in TArray< TScriptInterface<T> >", [this]
        {
            TestCommon([](UGCTestViewModel* ViewModel, auto Obj)
            {
                ViewModel->SetStruct({ { Obj } });
            });
        });
    });

    Describe("TSet", [this]
    {
        It("Should keep object stored in TSet<UObject*>", [this]
        {
            TestCommon([](UGCTestViewModel* ViewModel, auto Obj)
            {
                ViewModel->SetPointerSet({ Obj });
            });
        });

        It("Should keep object stored in TSet< UStruct::UObject* >", [this]
        {
            TestCommon([](UGCTestViewModel* ViewModel, auto Obj)
            {
                ViewModel->SetStructSet({ { Obj }, {} });
            });
        });
    });

    Describe("Derived", [this]
    {
        It("Should keep object stored in base field of Derived ViewModel", [this]
        {
            TestDerived([](UGCTestDerivedViewModel* ViewModel, auto Obj)
            {
                ViewModel->SetPointer({ Obj });
            });
        });

        It("Should keep object stored in own field of Derived ViewModel", [this]
        {
            TestDerived([](UGCTestDerivedViewModel* ViewModel, auto Obj)
            {
                ViewModel->SetDerivedPointer({ Obj });
            });
        });
    });
}


void FViewModelGCSpec::TestCommon(TFunctionRef<void(UGCTestViewModel*, UGCTestObject*)> Action, bool bExpectAlive)
{
    TestImpl<UGCTestViewModel>(Action, bExpectAlive);
}

void FViewModelGCSpec::TestDerived(TFunctionRef<void(UGCTestDerivedViewModel*, UGCTestObject*)> Action, bool bExpectAlive)
{
    TestImpl<UGCTestDerivedViewModel>(Action, bExpectAlive);
}

template <typename TViewModel, typename TAction>
void FViewModelGCSpec::TestImpl(TAction Action, bool bExpectAlive)
{
    TStrongObjectPtr<TViewModel> VM{ NewObject<TViewModel>() };
    TWeakObjectPtr<UGCTestObject> Ptr = NewObject<UGCTestObject>();

    TestTrue("Object Alive", Ptr.IsValid());

    Action(VM.Get(), Ptr.Get());

    CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);

    if (bExpectAlive)
    {
        TestTrue("Object Alive", Ptr.IsValid());
    }
    else
    {
        TestFalse("Object Alive", Ptr.IsValid());
    }
}