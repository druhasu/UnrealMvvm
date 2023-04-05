// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Misc/AutomationTest.h"

#include "TestBaseView.h"
#include "TestBaseViewModel.h"
#include "Mvvm/MvvmBlueprintLibrary.h"

#include "TempWorldHelper.h"

BEGIN_DEFINE_SPEC(FBaseViewSpec, "UnrealMvvm.BaseView", EAutomationTestFlags::ClientContext | EAutomationTestFlags::EditorContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)

UTestBaseViewBlueprint* CreateBlueprintBasedView(UWorld* World, bool bWithBindings = true) const;

END_DEFINE_SPEC(FBaseViewSpec)

void FBaseViewSpec::Define()
{
    Describe("UBaseView derived", [this]
    {
        It("Should Construct Widget", [this]
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

        It("Should Receive Changes When Constructed And Then Set ViewModel", [this]
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

        It("Should Receive Changes When Set ViewModel And Then Constructed", [this]
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

        It("Should Not Receive Changes When Only Constructed", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseView* View = CreateWidget<UTestBaseView>(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            TSharedPtr<SWidget> SWidgetPtr = View->TakeWidget();

            TestEqual("MyValue is non default", View->MyValue, 0);

            ViewModel->SetIntValue(1);
            TestEqual("MyValue is non default", View->MyValue, 0);
        });

        It("Should Not Receive Changes When Only Set ViewModel", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseView* View = CreateWidget<UTestBaseView>(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            View->SetViewModel(ViewModel);

            TestEqual("MyValue is non default", View->MyValue, 0);

            ViewModel->SetIntValue(1);
            TestEqual("MyValue is non default", View->MyValue, 0);
        });

        It("Should Not Receive Changes After Destructed", [this]
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

        It("Should Not Receive Changes After Unset ViewModel", [this]
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

        It("Should Return Untyped ViewModel", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseView* View = CreateWidget<UTestBaseView>(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            View->SetViewModel(ViewModel);

            TestEqual("Untyped ViewModel", View->GetUntypedViewModel(), StaticCast<UBaseViewModel*>(ViewModel));
        });

        It("Should Call OnViewModelChanged", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseView* View = CreateWidget<UTestBaseView>(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            View->SetViewModel(ViewModel);
            TestNull("OldViewModel", View->OldViewModel);
            TestEqual("NewViewModel", View->NewViewModel, ViewModel);

            View->SetViewModel(nullptr);
            TestEqual("OldViewModel", View->OldViewModel, ViewModel);
            TestNull("NewViewModel", View->NewViewModel);
        });

        It("Should Support Views Without Bindings", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseViewNoBind* View = CreateWidget<UTestBaseViewNoBind>(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            View->SetViewModel(ViewModel);
            TSharedPtr<SWidget> SWidgetPtr = View->TakeWidget();
        });
    });

    Describe("TBaseView<> derived", [this]
    {
        It("Should Construct Widget", [this]
        {
            // this test just checks our assumptions about what leads to NativeDestruct and NativeConstruct calls
            FTempWorldHelper Helper;

            UTestBaseViewPure* View = CreateWidget<UTestBaseViewPure>(Helper.World);
            TestNotNull("View not created", View);

            TSharedPtr<SWidget> SWidgetPtr = View->TakeWidget();
            TestTrue("View not constructed", View->IsConstructed());

            SWidgetPtr.Reset();
            TestFalse("View not destructed", View->IsConstructed());
        });

        It("Should Receive Changes When Constructed And Then Set ViewModel", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseViewPure* View = CreateWidget<UTestBaseViewPure>(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            ViewModel->SetIntValue(1);

            TestEqual("MyValue is non default", View->MyValue, 0);

            TSharedPtr<SWidget> SWidgetPtr = View->TakeWidget();
            View->SetViewModel(ViewModel);
            TestEqual("MyValue is not updated", View->MyValue, 1);

            ViewModel->SetIntValue(2);
            TestEqual("MyValue is not updated", View->MyValue, 2);
        });

        It("Should Receive Changes When Set ViewModel And Then Constructed", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseViewPure* View = CreateWidget<UTestBaseViewPure>(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            ViewModel->SetIntValue(1);

            TestEqual("MyValue is non default", View->MyValue, 0);

            View->SetViewModel(ViewModel);
            TSharedPtr<SWidget> SWidgetPtr = View->TakeWidget();
            TestEqual("MyValue is non default", View->MyValue, 1);

            ViewModel->SetIntValue(2);
            TestEqual("MyValue is not updated", View->MyValue, 2);
        });

        It("Should Not Receive Changes When Only Constructed", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseViewPure* View = CreateWidget<UTestBaseViewPure>(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            TSharedPtr<SWidget> SWidgetPtr = View->TakeWidget();

            TestEqual("MyValue is non default", View->MyValue, 0);

            ViewModel->SetIntValue(1);
            TestEqual("MyValue is non default", View->MyValue, 0);
        });

        It("Should Not Receive Changes When Only Set ViewModel", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseViewPure* View = CreateWidget<UTestBaseViewPure>(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            View->SetViewModel(ViewModel);

            TestEqual("MyValue is non default", View->MyValue, 0);

            ViewModel->SetIntValue(1);
            TestEqual("MyValue is non default", View->MyValue, 0);
        });

        It("Should Not Receive Changes After Destructed", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseViewPure* View = CreateWidget<UTestBaseViewPure>(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            TSharedPtr<SWidget> SWidgetPtr = View->TakeWidget();
            View->SetViewModel(ViewModel);

            ViewModel->SetIntValue(1);

            SWidgetPtr.Reset();
            ViewModel->SetIntValue(2);
            TestEqual("MyValue was wrongly updated", View->MyValue, 1);
        });

        It("Should Not Receive Changes After Unset ViewModel", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseViewPure* View = CreateWidget<UTestBaseViewPure>(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            TSharedPtr<SWidget> SWidgetPtr = View->TakeWidget();
            View->SetViewModel(ViewModel);

            ViewModel->SetIntValue(1);

            View->SetViewModel(nullptr);
            ViewModel->SetIntValue(2);
            TestEqual("MyValue was wrongly updated", View->MyValue, 1);
        });

        It("Should Return Untyped ViewModel", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseViewPure* View = CreateWidget<UTestBaseViewPure>(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            View->SetViewModel(ViewModel);

            TestEqual("Untyped ViewModel", UMvvmBlueprintLibrary::GetViewModel(View), StaticCast<UBaseViewModel*>(ViewModel));
        });

        It("Should Call OnViewModelChanged", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseViewPure* View = CreateWidget<UTestBaseViewPure>(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            View->SetViewModel(ViewModel);
            TestNull("OldViewModel", View->OldViewModel);
            TestEqual("NewViewModel", View->NewViewModel, ViewModel);

            View->SetViewModel(nullptr);
            TestEqual("OldViewModel", View->OldViewModel, ViewModel);
            TestNull("NewViewModel", View->NewViewModel);
        });

        It("Should Support Views Without Bindings", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseViewPureNoBind* View = CreateWidget<UTestBaseViewPureNoBind>(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            View->SetViewModel(ViewModel);
            TSharedPtr<SWidget> SWidgetPtr = View->TakeWidget();
        });
    });

    Describe("Blueprint only", [this]
    {
        It("Should Construct Widget", [this]
        {
            // this test just checks our assumptions about what leads to NativeDestruct and NativeConstruct calls
            FTempWorldHelper Helper;

            UTestBaseViewBlueprint* View = CreateBlueprintBasedView(Helper.World);
            TestNotNull("View not created", View);

            TSharedPtr<SWidget> SWidgetPtr = View->TakeWidget();
            TestTrue("View not constructed", View->IsConstructed());

            SWidgetPtr.Reset();
            TestFalse("View not destructed", View->IsConstructed());
        });

        It("Should Receive Changes When Constructed And Then Set ViewModel", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseViewBlueprint* View = CreateBlueprintBasedView(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            ViewModel->SetIntValue(1);

            TestEqual("MyValue is non default", View->MyValue, 0);

            TSharedPtr<SWidget> SWidgetPtr = View->TakeWidget();
            UMvvmBlueprintLibrary::SetViewModel(View, ViewModel);
            TestEqual("MyValue is not updated", View->MyValue, 1);

            ViewModel->SetIntValue(2);
            TestEqual("MyValue is not updated", View->MyValue, 2);
        });

        It("Should Receive Changes When Set ViewModel And Then Constructed", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseViewBlueprint* View = CreateBlueprintBasedView(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            ViewModel->SetIntValue(1);

            TestEqual("MyValue is non default", View->MyValue, 0);

            UMvvmBlueprintLibrary::SetViewModel(View, ViewModel);
            TSharedPtr<SWidget> SWidgetPtr = View->TakeWidget();
            TestEqual("MyValue is non default", View->MyValue, 1);

            ViewModel->SetIntValue(2);
            TestEqual("MyValue is not updated", View->MyValue, 2);
        });

        It("Should Not Receive Changes When Only Constructed", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseViewBlueprint* View = CreateBlueprintBasedView(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            TSharedPtr<SWidget> SWidgetPtr = View->TakeWidget();

            TestEqual("MyValue is non default", View->MyValue, 0);

            ViewModel->SetIntValue(1);
            TestEqual("MyValue is non default", View->MyValue, 0);
        });

        It("Should Not Receive Changes When Only Set ViewModel", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseViewBlueprint* View = CreateBlueprintBasedView(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            UMvvmBlueprintLibrary::SetViewModel(View, ViewModel);

            TestEqual("MyValue is non default", View->MyValue, 0);

            ViewModel->SetIntValue(1);
            TestEqual("MyValue is non default", View->MyValue, 0);
        });

        It("Should Not Receive Changes After Destructed", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseViewBlueprint* View = CreateBlueprintBasedView(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            TSharedPtr<SWidget> SWidgetPtr = View->TakeWidget();
            UMvvmBlueprintLibrary::SetViewModel(View, ViewModel);

            ViewModel->SetIntValue(1);

            SWidgetPtr.Reset();
            ViewModel->SetIntValue(2);
            TestEqual("MyValue was wrongly updated", View->MyValue, 1);
        });

        It("Should Not Receive Changes After Unset ViewModel", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseViewBlueprint* View = CreateBlueprintBasedView(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            TSharedPtr<SWidget> SWidgetPtr = View->TakeWidget();
            UMvvmBlueprintLibrary::SetViewModel(View, ViewModel);

            ViewModel->SetIntValue(1);

            UMvvmBlueprintLibrary::SetViewModel(View, nullptr);
            ViewModel->SetIntValue(2);
            TestEqual("MyValue was wrongly updated", View->MyValue, 1);
        });

        It("Should Return Untyped ViewModel", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseViewBlueprint* View = CreateBlueprintBasedView(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            UMvvmBlueprintLibrary::SetViewModel(View, ViewModel);

            TestEqual("Untyped ViewModel", UMvvmBlueprintLibrary::GetViewModel(View), StaticCast<UBaseViewModel*>(ViewModel));
        });

        It("Should Call OnViewModelChanged", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseViewBlueprint* View = CreateBlueprintBasedView(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            UMvvmBlueprintLibrary::SetViewModel(View, ViewModel);
            TestNull("OldViewModel", View->OldViewModel);
            TestEqual("NewViewModel", View->NewViewModel, ViewModel);

            UMvvmBlueprintLibrary::SetViewModel(View, nullptr);
            TestEqual("OldViewModel", View->OldViewModel, ViewModel);
            TestNull("NewViewModel", View->NewViewModel);
        });

        It("Should Support Views Without Bindings", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseViewBlueprint* View = CreateBlueprintBasedView(Helper.World, false);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            UMvvmBlueprintLibrary::SetViewModel(View, ViewModel);
            TSharedPtr<SWidget> SWidgetPtr = View->TakeWidget();
        });

        It("Should Support Nested View", [this]
        {
            FTempWorldHelper Helper;

            UClass* WidgetClass = StaticLoadClass(UUserWidget::StaticClass(), nullptr, TEXT("/UnrealMvvmTests/BP_HostWidget.BP_HostWidget_C"));
            UUserWidget* Widget = CreateWidget<UUserWidget>(Helper.World, WidgetClass);

            auto WidgetRef = Widget->TakeWidget();
            UMvvmBlueprintLibrary::SetViewModel(Widget, NewObject<UTestBaseViewModel>());
        });
    });
}

UTestBaseViewBlueprint* FBaseViewSpec::CreateBlueprintBasedView(UWorld* World, bool bWithBindings) const
{
    UClass* WidgetClass = bWithBindings ?
        StaticLoadClass(UUserWidget::StaticClass(), nullptr, TEXT("/UnrealMvvmTests/BP_TestBaseView.BP_TestBaseView_C")) :
        StaticLoadClass(UUserWidget::StaticClass(), nullptr, TEXT("/UnrealMvvmTests/BP_TestBaseView_NoBind.BP_TestBaseView_NoBind_C"));

    return CreateWidget<UTestBaseViewBlueprint>(World, WidgetClass);
}