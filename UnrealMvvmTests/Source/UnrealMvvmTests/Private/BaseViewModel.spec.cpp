// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Misc/AutomationTest.h"

#include "PropertyChangeCounter.h"

#include "TestBaseViewModel.h"
#include "TestCompareViewModel.h"

BEGIN_DEFINE_SPEC(FBaseViewModelSpec, "UnrealMvvm.BaseViewModel", EAutomationTestFlags::ClientContext | EAutomationTestFlags::EditorContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
using FChangeDelegate = UBaseViewModel::FPropertyChangedDelegate::FDelegate;
END_DEFINE_SPEC(FBaseViewModelSpec)

void FBaseViewModelSpec::Define()
{
    Describe("HasConnectedViews", [this]()
    {
        It("Should Have No Connected Views Before Subscribe", [this]()
        {
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            TestFalse("Some View connected", ViewModel->HasConnectedViews());
        });

        It("Should Have Connected Views After Subscribe", [this]()
        {
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            ViewModel->Subscribe(FChangeDelegate::CreateLambda([](auto) {}));

            TestTrue("No views connected", ViewModel->HasConnectedViews());
        });

        It("Should Have No Connected Views After Unsubscribe", [this]()
        {
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            FDelegateHandle Handle = ViewModel->Subscribe(FChangeDelegate::CreateLambda([](auto) {}));
            ViewModel->Unsubscribe(Handle);

            TestFalse("Some View still connected", ViewModel->HasConnectedViews());
        });

        It("Should Receive StatusChanged After First Subscribe", [this]()
        {
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            ViewModel->Subscribe(FChangeDelegate::CreateLambda([](auto) {}));

            TestTrue("No status received", ViewModel->LastSubscriptionStatus.IsSet());
            TestTrue("Wrong status received", ViewModel->LastSubscriptionStatus.Get(false));
            ViewModel->LastSubscriptionStatus.Reset();

            ViewModel->Subscribe(FChangeDelegate::CreateLambda([](auto) {}));
            
            TestFalse("Extra status received", ViewModel->LastSubscriptionStatus.IsSet());
        });

        It("Should Receive StatusChanged After Last Unsubscribe", [this]()
        {
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            FDelegateHandle Handle1 = ViewModel->Subscribe(FChangeDelegate::CreateLambda([](auto) {}));
            FDelegateHandle Handle2 = ViewModel->Subscribe(FChangeDelegate::CreateLambda([](auto) {}));
            ViewModel->LastSubscriptionStatus.Reset();

            ViewModel->Unsubscribe(Handle1);
            TestFalse("Extra status received", ViewModel->LastSubscriptionStatus.IsSet());
            ViewModel->LastSubscriptionStatus.Reset();

            ViewModel->Unsubscribe(Handle2);
            TestTrue("No status received", ViewModel->LastSubscriptionStatus.IsSet());
            TestFalse("Wrong status received", ViewModel->LastSubscriptionStatus.Get(true));
        });
    });

    Describe("RaiseChanged", [this]
    {
        It("Should notify view about single property", [this]
        {
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            FPropertyChangeCounter Counter(ViewModel);

            ViewModel->RaiseSingleChange();

            TestEqual("Changed property", Counter[UTestBaseViewModel::IntValueProperty()], 1);
        });

        It("Should notify view about multiple property", [this]
        {
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            FPropertyChangeCounter Counter(ViewModel);

            ViewModel->RaiseMultipleChange();

            TestEqual("IntValueProperty changed", Counter[UTestBaseViewModel::IntValueProperty()], 1);
            TestEqual("FloatValueProperty changed", Counter[UTestBaseViewModel::FloatValueProperty()], 1);
        });
    });

    Describe("Compare on Set", [this]
    {
        It("Should compare when setting int32", [this]
        {
            UTestCompareViewModel* ViewModel = NewObject<UTestCompareViewModel>();
            FPropertyChangeCounter Counter(ViewModel);
            int32 Value = 1;

            TestEqual("Changes", Counter[UTestCompareViewModel::IntValueProperty()], 0);

            ViewModel->SetIntValue(Value);
            TestEqual("Changes", Counter[UTestCompareViewModel::IntValueProperty()], 1);

            ViewModel->SetIntValue(Value);
            TestEqual("Changes", Counter[UTestCompareViewModel::IntValueProperty()], 1);
        });

        It("Should compare when setting pointer", [this]
        {
            UTestCompareViewModel* ViewModel = NewObject<UTestCompareViewModel>();
            FPropertyChangeCounter Counter(ViewModel);
            UObject* Value = ViewModel;

            TestEqual("Changes", Counter[UTestCompareViewModel::ObjectValueProperty()], 0);

            ViewModel->SetObjectValue(Value);
            TestEqual("Changes", Counter[UTestCompareViewModel::ObjectValueProperty()], 1);

            ViewModel->SetObjectValue(Value);
            TestEqual("Changes", Counter[UTestCompareViewModel::ObjectValueProperty()], 1);
        });

        It("Should compare when setting Comparable struct", [this]
        {
            UTestCompareViewModel* ViewModel = NewObject<UTestCompareViewModel>();
            FPropertyChangeCounter Counter(ViewModel);
            FTestComparableStruct Value{ 1 };

            static_assert(UnrealMvvm_Impl::TCanCompareHelper<FTestComparableStruct>::Value, "Cannot compare FTestComparableStruct");

            TestEqual("Changes", Counter[UTestCompareViewModel::ComparableStructValueProperty()], 0);

            ViewModel->SetComparableStructValue(Value);
            TestEqual("Changes", Counter[UTestCompareViewModel::ComparableStructValueProperty()], 1);

            ViewModel->SetComparableStructValue(Value);
            TestEqual("Changes", Counter[UTestCompareViewModel::ComparableStructValueProperty()], 1);
        });

        It("Should not compare when setting NonComparable struct", [this]
        {
            UTestCompareViewModel* ViewModel = NewObject<UTestCompareViewModel>();
            FPropertyChangeCounter Counter(ViewModel);
            FTestNonComparableStruct Value{ 1 };

            static_assert(!UnrealMvvm_Impl::TCanCompareHelper<FTestNonComparableStruct>::Value, "Can compare FTestNonComparableStruct");

            TestEqual("Changes", Counter[UTestCompareViewModel::NonComparableStructValueProperty()], 0);

            ViewModel->SetNonComparableStructValue(Value);
            TestEqual("Changes", Counter[UTestCompareViewModel::NonComparableStructValueProperty()], 1);

            ViewModel->SetNonComparableStructValue(Value);
            TestEqual("Changes", Counter[UTestCompareViewModel::NonComparableStructValueProperty()], 2);
        });

        It("Should not compare when setting Comparable struct with disabled comparison", [this]
        {
            UTestCompareViewModel* ViewModel = NewObject<UTestCompareViewModel>();
            FPropertyChangeCounter Counter(ViewModel);
            FTestComparableDisabledStruct Value{ 1 };

            static_assert(UnrealMvvm_Impl::TCanCompareHelper<FTestComparableDisabledStruct>::Value, "Cannot compare FTestComparableDisabledStruct");

            TestEqual("Changes", Counter[UTestCompareViewModel::ComparableDisabledStructValueProperty()], 0);

            ViewModel->SetComparableDisabledStructValue(Value);
            TestEqual("Changes", Counter[UTestCompareViewModel::ComparableDisabledStructValueProperty()], 1);

            ViewModel->SetComparableDisabledStructValue(Value);
            TestEqual("Changes", Counter[UTestCompareViewModel::ComparableDisabledStructValueProperty()], 2);
        });


        It("Should compare when setting Comparable struct Array", [this]
        {
            UTestCompareViewModel* ViewModel = NewObject<UTestCompareViewModel>();
            FPropertyChangeCounter Counter(ViewModel);
            TArray<FTestComparableStruct> Value{ { 1 }, { 2 } };

            static_assert(UnrealMvvm_Impl::TCanCompareHelper<TArray<FTestComparableStruct>>::Value, "Cannot compare TArray<FTestComparableStruct>");

            TestEqual("Changes", Counter[UTestCompareViewModel::ComparableStructArrayValueProperty()], 0);

            ViewModel->SetComparableStructArrayValue(Value);
            TestEqual("Changes", Counter[UTestCompareViewModel::ComparableStructArrayValueProperty()], 1);

            ViewModel->SetComparableStructArrayValue(Value);
            TestEqual("Changes", Counter[UTestCompareViewModel::ComparableStructArrayValueProperty()], 1);
        });

        It("Should not compare when setting NonComparable struct Array", [this]
        {
            UTestCompareViewModel* ViewModel = NewObject<UTestCompareViewModel>();
            FPropertyChangeCounter Counter(ViewModel);
            TArray<FTestNonComparableStruct> Value{ { 1 }, { 2 } };

            static_assert(!UnrealMvvm_Impl::TCanCompareHelper<TArray<FTestNonComparableStruct>>::Value, "Can compare TArray<FTestNonComparableStruct>");

            TestEqual("Changes", Counter[UTestCompareViewModel::NonComparableStructArrayValueProperty()], 0);

            ViewModel->SetNonComparableStructArrayValue(Value);
            TestEqual("Changes", Counter[UTestCompareViewModel::NonComparableStructArrayValueProperty()], 1);

            ViewModel->SetNonComparableStructArrayValue(Value);
            TestEqual("Changes", Counter[UTestCompareViewModel::NonComparableStructArrayValueProperty()], 2);
        });


        It("Should compare when setting Comparable struct Optional", [this]
        {
            UTestCompareViewModel* ViewModel = NewObject<UTestCompareViewModel>();
            FPropertyChangeCounter Counter(ViewModel);
            TOptional<FTestComparableStruct> Value{ FTestComparableStruct{ 1 } };

            static_assert(UnrealMvvm_Impl::TCanCompareHelper<TOptional<FTestComparableStruct>>::Value, "Cannot compare TOptional<FTestComparableStruct>");

            TestEqual("Changes", Counter[UTestCompareViewModel::ComparableStructOptionalValueProperty()], 0);

            ViewModel->SetComparableStructOptionalValue(Value);
            TestEqual("Changes", Counter[UTestCompareViewModel::ComparableStructOptionalValueProperty()], 1);

            ViewModel->SetComparableStructOptionalValue(Value);
            TestEqual("Changes", Counter[UTestCompareViewModel::ComparableStructOptionalValueProperty()], 1);
        });

        It("Should not compare when setting NonComparable struct Optional", [this]
        {
            UTestCompareViewModel* ViewModel = NewObject<UTestCompareViewModel>();
            FPropertyChangeCounter Counter(ViewModel);
            TOptional<FTestNonComparableStruct> Value{ FTestNonComparableStruct{ 1 } };

            static_assert(!UnrealMvvm_Impl::TCanCompareHelper<TOptional<FTestNonComparableStruct>>::Value, "Can compare TOptional<FTestNonComparableStruct>");

            TestEqual("Changes", Counter[UTestCompareViewModel::NonComparableStructOptionalValueProperty()], 0);

            ViewModel->SetNonComparableStructOptionalValue(Value);
            TestEqual("Changes", Counter[UTestCompareViewModel::NonComparableStructOptionalValueProperty()], 1);

            ViewModel->SetNonComparableStructOptionalValue(Value);
            TestEqual("Changes", Counter[UTestCompareViewModel::NonComparableStructOptionalValueProperty()], 2);
        });


        It("Should compare when setting Comparable struct Optional Optional", [this]
        {
            UTestCompareViewModel* ViewModel = NewObject<UTestCompareViewModel>();
            FPropertyChangeCounter Counter(ViewModel);
            TOptional<TOptional<FTestComparableStruct>> Value{ TOptional<FTestComparableStruct>{ FTestComparableStruct{ 1 } } };

            static_assert(UnrealMvvm_Impl::TCanCompareHelper<TOptional<TOptional<FTestComparableStruct>>>::Value, "Cannot compare TOptional<TOptional<FTestComparableStruct>>");

            TestEqual("Changes", Counter[UTestCompareViewModel::ComparableStructOptionalOptionalValueProperty()], 0);

            ViewModel->SetComparableStructOptionalOptionalValue(Value);
            TestEqual("Changes", Counter[UTestCompareViewModel::ComparableStructOptionalOptionalValueProperty()], 1);

            ViewModel->SetComparableStructOptionalOptionalValue(Value);
            TestEqual("Changes", Counter[UTestCompareViewModel::ComparableStructOptionalOptionalValueProperty()], 1);
        });

        It("Should not compare when setting NonComparable struct Optional Optional", [this]
        {
            UTestCompareViewModel* ViewModel = NewObject<UTestCompareViewModel>();
            FPropertyChangeCounter Counter(ViewModel);
            TOptional<TOptional<FTestNonComparableStruct>> Value{ TOptional<FTestNonComparableStruct>{ FTestNonComparableStruct{ 1 } } };

            static_assert(!UnrealMvvm_Impl::TCanCompareHelper<TOptional<TOptional<FTestNonComparableStruct>>>::Value, "Can compare TOptional<TOptional<FTestNonComparableStruct>>");

            TestEqual("Changes", Counter[UTestCompareViewModel::NonComparableStructOptionalOptionalValueProperty()], 0);

            ViewModel->SetNonComparableStructOptionalOptionalValue(Value);
            TestEqual("Changes", Counter[UTestCompareViewModel::NonComparableStructOptionalOptionalValueProperty()], 1);

            ViewModel->SetNonComparableStructOptionalOptionalValue(Value);
            TestEqual("Changes", Counter[UTestCompareViewModel::NonComparableStructOptionalOptionalValueProperty()], 2);
        });
    });
}
