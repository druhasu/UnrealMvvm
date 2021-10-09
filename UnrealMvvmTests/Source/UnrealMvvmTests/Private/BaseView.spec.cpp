// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Misc/AutomationTest.h"

#include "TestBaseView.h"
#include "TestBaseViewModel.h"

#include "TempWorldHelper.h"

BEGIN_DEFINE_SPEC(BaseViewSpec, "UnrealMvvm.BaseView", EAutomationTestFlags::ClientContext | EAutomationTestFlags::EditorContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
END_DEFINE_SPEC(BaseViewSpec)

void BaseViewSpec::Define()
{
    It("Should Construct Widget", [this]()
    {
        // this test just checks our assumptions about what leads to NativeDestruct and NativeConstruct calls
        FTempWorldHelper Helper;

        UTestBaseView* View = CreateWidget<UTestBaseView>(Helper.World);
        TestNotNull("View not created", View);

        TSharedPtr<SWidget> SWidgetPtr = View->TakeWidget();
        TestTrue("View not constructed", View->IsConstructed());

        SWidgetPtr.Reset();
        TestFalse("View not destructed", View->IsConstructed());
    });

    It("Should Receive Changes When Constructed And Then Set ViewModel", [this]()
    {
        FTempWorldHelper Helper;

        UTestBaseView* View = CreateWidget<UTestBaseView>(Helper.World);
        UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
        ViewModel->SetIntValue(1);

        TestEqual("MyValue is non default", View->MyValue, 0);

        TSharedPtr<SWidget> SWidgetPtr = View->TakeWidget();
        View->SetViewModel(ViewModel);
        TestEqual("MyValue is not updated", View->MyValue, 1);

        ViewModel->SetIntValue(2);
        TestEqual("MyValue is not updated", View->MyValue, 2);
    });

    It("Should Receive Changes When Set ViewModel And Then Constructed", [this]()
    {
        FTempWorldHelper Helper;

        UTestBaseView* View = CreateWidget<UTestBaseView>(Helper.World);
        UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
        ViewModel->SetIntValue(1);

        TestEqual("MyValue is non default", View->MyValue, 0);

        View->SetViewModel(ViewModel);
        TSharedPtr<SWidget> SWidgetPtr = View->TakeWidget();
        TestEqual("MyValue is non default", View->MyValue, 1);

        ViewModel->SetIntValue(2);
        TestEqual("MyValue is not updated", View->MyValue, 2);
    });

    It("Should Not Receive Changes When Only Constructed", [this]()
    {
        FTempWorldHelper Helper;

        UTestBaseView* View = CreateWidget<UTestBaseView>(Helper.World);
        UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

        TSharedPtr<SWidget> SWidgetPtr = View->TakeWidget();

        TestEqual("MyValue is non default", View->MyValue, 0);

        ViewModel->SetIntValue(1);
        TestEqual("MyValue is non default", View->MyValue, 0);
    });

    It("Should Not Receive Changes When Only Set ViewModel", [this]()
    {
        FTempWorldHelper Helper;

        UTestBaseView* View = CreateWidget<UTestBaseView>(Helper.World);
        UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

        View->SetViewModel(ViewModel);

        TestEqual("MyValue is non default", View->MyValue, 0);

        ViewModel->SetIntValue(1);
        TestEqual("MyValue is non default", View->MyValue, 0);
    });

    It("Should Not Receive Changes After Destructed", [this]()
    {
        FTempWorldHelper Helper;

        UTestBaseView* View = CreateWidget<UTestBaseView>(Helper.World);
        UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

        TSharedPtr<SWidget> SWidgetPtr = View->TakeWidget();
        View->SetViewModel(ViewModel);

        ViewModel->SetIntValue(1);

        SWidgetPtr.Reset();
        ViewModel->SetIntValue(2);
        TestEqual("MyValue was wrongly updated", View->MyValue, 1);
    });

    It("Should Not Receive Changes After Unset ViewModel", [this]()
    {
        FTempWorldHelper Helper;

        UTestBaseView* View = CreateWidget<UTestBaseView>(Helper.World);
        UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

        TSharedPtr<SWidget> SWidgetPtr = View->TakeWidget();
        View->SetViewModel(ViewModel);

        ViewModel->SetIntValue(1);

        View->SetViewModel(nullptr);
        ViewModel->SetIntValue(2);
        TestEqual("MyValue was wrongly updated", View->MyValue, 1);
    });
}