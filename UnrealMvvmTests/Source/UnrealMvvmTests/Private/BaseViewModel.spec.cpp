// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Misc/AutomationTest.h"

#include "TestBaseViewModel.h"

BEGIN_DEFINE_SPEC(BaseViewModelSpec, "UnrealMvvm.BaseViewModel", EAutomationTestFlags::ClientContext | EAutomationTestFlags::EditorContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
END_DEFINE_SPEC(BaseViewModelSpec)

using FChangeDelegate = UBaseViewModel::FPropertyChangedDelegate::FDelegate;

void BaseViewModelSpec::Define()
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
}