// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Misc/AutomationTest.h"

#include "TestBaseWidgetView.h"
#include "TestBaseViewModel.h"
#include "BindingWorkerTestViewModel.h"
#include "Mvvm/MvvmBlueprintLibrary.h"
#include "Slate/SObjectWidget.h"

#include "TempWorldHelper.h"

BEGIN_DEFINE_SPEC(FBaseWidgetViewSpec, "UnrealMvvm.BaseWidgetView", EAutomationTestFlags::ClientContext | EAutomationTestFlags::EditorContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)

UTestBaseWidgetViewPureNoBind* CreateNativeBasedView(UWorld* World, bool bWithBindings = true) const;
UTestBaseWidgetViewBlueprint* CreateBlueprintBasedView(UWorld* World, bool bWithBindings = true) const;

END_DEFINE_SPEC(FBaseWidgetViewSpec)

void FBaseWidgetViewSpec::Define()
{
    Describe("TBaseView<> derived Native", [this]
    {
        It("Should Construct Widget", [this]
        {
            // this test just checks our assumptions about what leads to NativeDestruct and NativeConstruct calls
            FTempWorldHelper Helper;

            UTestBaseWidgetViewPure* View = CreateWidget<UTestBaseWidgetViewPure>(Helper.World);
            TestNotNull("View not created", View);

            TSharedPtr<SWidget> SWidgetPtr = View->TakeWidget();
            TestTrue("View not constructed", View->IsConstructed());

            SWidgetPtr.Reset();
            TestFalse("View not destructed", View->IsConstructed());
        });

        It("Should Receive Changes When Constructed And Then Set ViewModel", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseWidgetViewPure* View = CreateWidget<UTestBaseWidgetViewPure>(Helper.World);
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

            UTestBaseWidgetViewPure* View = CreateWidget<UTestBaseWidgetViewPure>(Helper.World);
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

            UTestBaseWidgetViewPure* View = CreateWidget<UTestBaseWidgetViewPure>(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            TSharedPtr<SWidget> SWidgetPtr = View->TakeWidget();

            TestEqual("MyValue is non default", View->MyValue, 0);

            ViewModel->SetIntValue(1);
            TestEqual("MyValue is non default", View->MyValue, 0);
        });

        It("Should Not Receive Changes When Only Set ViewModel", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseWidgetViewPure* View = CreateWidget<UTestBaseWidgetViewPure>(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            View->SetViewModel(ViewModel);

            TestEqual("MyValue is non default", View->MyValue, 0);

            ViewModel->SetIntValue(1);
            TestEqual("MyValue is non default", View->MyValue, 0);
        });

        It("Should Not Receive Changes After Destructed", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseWidgetViewPure* View = CreateWidget<UTestBaseWidgetViewPure>(Helper.World);
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

            UTestBaseWidgetViewPure* View = CreateWidget<UTestBaseWidgetViewPure>(Helper.World);
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

            UTestBaseWidgetViewPure* View = CreateWidget<UTestBaseWidgetViewPure>(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            View->SetViewModel(ViewModel);

            TestEqual("Untyped ViewModel", UMvvmBlueprintLibrary::GetViewModel(View), StaticCast<UBaseViewModel*>(ViewModel));
        });

        It("Should Call OnViewModelChanged", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseWidgetViewPure* View = CreateWidget<UTestBaseWidgetViewPure>(Helper.World);
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

            UTestBaseWidgetViewPureNoBind* View = CreateWidget<UTestBaseWidgetViewPureNoBind>(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            View->SetViewModel(ViewModel);
            TSharedPtr<SWidget> SWidgetPtr = View->TakeWidget();
        });

        It("Should Subscribe Once If ViewModel Set During Construction", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseWidgetViewPure* View = CreateWidget<UTestBaseWidgetViewPure>(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            // when using CommonUI, this is how ViewModels may be injected into widgets created via UserWidgetPool
            TSharedPtr<SWidget> SWidgetPtr = View->TakeDerivedWidget([&](UUserWidget* Widget, TSharedRef<SWidget> Content)
            {
                View->SetViewModel(ViewModel);
                return SNew(SObjectWidget, Widget)[Content];
            });
            
            ViewModel->SetIntValue(1);

            SWidgetPtr.Reset();
            TestFalse("ViewModel has connected Views", ViewModel->HasConnectedViews());
        });

        It("Should report initialization when Constructed and then Set ViewModel", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseWidgetViewPure* View = CreateWidget<UTestBaseWidgetViewPure>(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            ViewModel->SetIntValue(1);

            View->IntValueChangedCallback = [&](auto)
            {
                TestTrue("IsInitializing by View", UMvvmBlueprintLibrary::IsInitializingProperty(View));
                TestTrue("IsInitializing by ViewModel", UMvvmBlueprintLibrary::IsInitializingProperty(ViewModel));
            };

            TestFalse("Is Initializing by View", UMvvmBlueprintLibrary::IsInitializingProperty(View));
            TestFalse("Is Initializing by ViewModel", UMvvmBlueprintLibrary::IsInitializingProperty(ViewModel));

            TSharedPtr<SWidget> SWidgetPtr = View->TakeWidget();
            View->SetViewModel(ViewModel);

            TestFalse("Is Initializing by View", UMvvmBlueprintLibrary::IsInitializingProperty(View));
            TestFalse("Is Initializing by ViewModel", UMvvmBlueprintLibrary::IsInitializingProperty(ViewModel));
        });

        It("Should report initialization when Set ViewModel and then Constructed", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseWidgetViewPure* View = CreateWidget<UTestBaseWidgetViewPure>(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            ViewModel->SetIntValue(1);

            View->IntValueChangedCallback = [&](auto)
            {
                TestTrue("IsInitializing by View", UMvvmBlueprintLibrary::IsInitializingProperty(View));
                TestTrue("IsInitializing by ViewModel", UMvvmBlueprintLibrary::IsInitializingProperty(ViewModel));
            };

            TestFalse("Is Initializing by View", UMvvmBlueprintLibrary::IsInitializingProperty(View));
            TestFalse("Is Initializing by ViewModel", UMvvmBlueprintLibrary::IsInitializingProperty(ViewModel));

            View->SetViewModel(ViewModel);
            TSharedPtr<SWidget> SWidgetPtr = View->TakeWidget();

            TestFalse("Is Initializing by View", UMvvmBlueprintLibrary::IsInitializingProperty(View));
            TestFalse("Is Initializing by ViewModel", UMvvmBlueprintLibrary::IsInitializingProperty(ViewModel));
        });

        It("Should report change", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseWidgetViewPure* View = CreateWidget<UTestBaseWidgetViewPure>(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            ViewModel->SetIntValue(1);

            TSharedPtr<SWidget> SWidgetPtr = View->TakeWidget();
            View->SetViewModel(ViewModel);

            TestFalse("IsChanging by View", UMvvmBlueprintLibrary::IsChangingProperty(View));
            TestFalse("IsChanging by ViewModel", UMvvmBlueprintLibrary::IsChangingProperty(ViewModel));

            View->IntValueChangedCallback = [&](auto)
            {
                TestTrue("IsChanging by View", UMvvmBlueprintLibrary::IsChangingProperty(View));
                TestTrue("IsChanging by ViewModel", UMvvmBlueprintLibrary::IsChangingProperty(ViewModel));
            };

            ViewModel->SetIntValue(2);

            TestFalse("IsChanging by View", UMvvmBlueprintLibrary::IsChangingProperty(View));
            TestFalse("IsChanging by ViewModel", UMvvmBlueprintLibrary::IsChangingProperty(ViewModel));
        });

        It("Should not report initialization during change", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseWidgetViewPure* View = CreateWidget<UTestBaseWidgetViewPure>(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            ViewModel->SetIntValue(1);

            TSharedPtr<SWidget> SWidgetPtr = View->TakeWidget();
            View->SetViewModel(ViewModel);

            View->IntValueChangedCallback = [&](auto)
            {
                TestFalse("Is Initializing by View", UMvvmBlueprintLibrary::IsInitializingProperty(View));
                TestFalse("Is Initializing by ViewModel", UMvvmBlueprintLibrary::IsInitializingProperty(ViewModel));
            };

            ViewModel->SetIntValue(2);
        });

        It("Should not report change during initialization when Set ViewModel and then Constructed", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseWidgetViewPure* View = CreateWidget<UTestBaseWidgetViewPure>(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            ViewModel->SetIntValue(1);

            View->IntValueChangedCallback = [&](auto)
            {
                TestFalse("IsChanging by View", UMvvmBlueprintLibrary::IsChangingProperty(View));
                TestFalse("IsChanging by ViewModel", UMvvmBlueprintLibrary::IsChangingProperty(ViewModel));
            };

            View->SetViewModel(ViewModel);
            TSharedPtr<SWidget> SWidgetPtr = View->TakeWidget();
        });

        It("Should not report change during initialization when Constructed and then Set ViewModel", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseWidgetViewPure* View = CreateWidget<UTestBaseWidgetViewPure>(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            ViewModel->SetIntValue(1);

            View->IntValueChangedCallback = [&](auto)
            {
                TestFalse("IsChanging by View", UMvvmBlueprintLibrary::IsChangingProperty(View));
                TestFalse("IsChanging by ViewModel", UMvvmBlueprintLibrary::IsChangingProperty(ViewModel));
            };

            TSharedPtr<SWidget> SWidgetPtr = View->TakeWidget();
            View->SetViewModel(ViewModel);
        });
    });

    Describe("TBaseView<> derived in Blueprint", [this]
    {
        It("Should Construct Widget", [this]
        {
            // this test just checks our assumptions about what leads to NativeDestruct and NativeConstruct calls
            FTempWorldHelper Helper;

            UTestBaseWidgetViewPureNoBind* View = CreateNativeBasedView(Helper.World);
            TestNotNull("View not created", View);

            TSharedPtr<SWidget> SWidgetPtr = View->TakeWidget();
            TestTrue("View not constructed", View->IsConstructed());

            SWidgetPtr.Reset();
            TestFalse("View not destructed", View->IsConstructed());
        });

        It("Should Receive Changes When Constructed And Then Set ViewModel", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseWidgetViewPureNoBind* View = CreateNativeBasedView(Helper.World);
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

            UTestBaseWidgetViewPureNoBind* View = CreateNativeBasedView(Helper.World);
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

            UTestBaseWidgetViewPureNoBind* View = CreateNativeBasedView(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            TSharedPtr<SWidget> SWidgetPtr = View->TakeWidget();

            TestEqual("MyValue is non default", View->MyValue, 0);

            ViewModel->SetIntValue(1);
            TestEqual("MyValue is non default", View->MyValue, 0);
        });

        It("Should Not Receive Changes When Only Set ViewModel", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseWidgetViewPureNoBind* View = CreateNativeBasedView(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            View->SetViewModel(ViewModel);

            TestEqual("MyValue is non default", View->MyValue, 0);

            ViewModel->SetIntValue(1);
            TestEqual("MyValue is non default", View->MyValue, 0);
        });

        It("Should Not Receive Changes After Destructed", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseWidgetViewPureNoBind* View = CreateNativeBasedView(Helper.World);
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

            UTestBaseWidgetViewPureNoBind* View = CreateNativeBasedView(Helper.World);
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

            UTestBaseWidgetViewPureNoBind* View = CreateNativeBasedView(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            View->SetViewModel(ViewModel);

            TestEqual("Untyped ViewModel", UMvvmBlueprintLibrary::GetViewModel(View), StaticCast<UBaseViewModel*>(ViewModel));
        });

        It("Should Call OnViewModelChanged", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseWidgetViewPureNoBind* View = CreateNativeBasedView(Helper.World);
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

            UTestBaseWidgetViewPureNoBind* View = CreateNativeBasedView(Helper.World, false);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            View->SetViewModel(ViewModel);
            TSharedPtr<SWidget> SWidgetPtr = View->TakeWidget();
        });

        It("Should Subscribe Once If ViewModel Set During Construction", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseWidgetViewPure* View = CreateWidget<UTestBaseWidgetViewPure>(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            // when using CommonUI, this is how ViewModels may be injected into widgets created via UserWidgetPool
            TSharedPtr<SWidget> SWidgetPtr = View->TakeDerivedWidget([&](UUserWidget* Widget, TSharedRef<SWidget> Content)
            {
                View->SetViewModel(ViewModel);
                return SNew(SObjectWidget, Widget)[Content];
            });

            ViewModel->SetIntValue(1);

            SWidgetPtr.Reset();
            TestFalse("ViewModel has connected Views", ViewModel->HasConnectedViews());
        });
    });

    Describe("Blueprint only", [this]
    {
        It("Should Construct Widget", [this]
        {
            // this test just checks our assumptions about what leads to NativeDestruct and NativeConstruct calls
            FTempWorldHelper Helper;

            UTestBaseWidgetViewBlueprint* View = CreateBlueprintBasedView(Helper.World);
            TestNotNull("View not created", View);

            TSharedPtr<SWidget> SWidgetPtr = View->TakeWidget();
            TestTrue("View not constructed", View->IsConstructed());

            SWidgetPtr.Reset();
            TestFalse("View not destructed", View->IsConstructed());
        });

        It("Should Receive Changes When Constructed And Then Set ViewModel", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseWidgetViewBlueprint* View = CreateBlueprintBasedView(Helper.World);
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

            UTestBaseWidgetViewBlueprint* View = CreateBlueprintBasedView(Helper.World);
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

            UTestBaseWidgetViewBlueprint* View = CreateBlueprintBasedView(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            TSharedPtr<SWidget> SWidgetPtr = View->TakeWidget();

            TestEqual("MyValue is non default", View->MyValue, 0);

            ViewModel->SetIntValue(1);
            TestEqual("MyValue is non default", View->MyValue, 0);
        });

        It("Should Not Receive Changes When Only Set ViewModel", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseWidgetViewBlueprint* View = CreateBlueprintBasedView(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            UMvvmBlueprintLibrary::SetViewModel(View, ViewModel);

            TestEqual("MyValue is non default", View->MyValue, 0);

            ViewModel->SetIntValue(1);
            TestEqual("MyValue is non default", View->MyValue, 0);
        });

        It("Should Not Receive Changes After Destructed", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseWidgetViewBlueprint* View = CreateBlueprintBasedView(Helper.World);
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

            UTestBaseWidgetViewBlueprint* View = CreateBlueprintBasedView(Helper.World);
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

            UTestBaseWidgetViewBlueprint* View = CreateBlueprintBasedView(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            UMvvmBlueprintLibrary::SetViewModel(View, ViewModel);

            TestEqual("Untyped ViewModel", UMvvmBlueprintLibrary::GetViewModel(View), StaticCast<UBaseViewModel*>(ViewModel));
        });

        It("Should Call OnViewModelChanged", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseWidgetViewBlueprint* View = CreateBlueprintBasedView(Helper.World);
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

            UTestBaseWidgetViewBlueprint* View = CreateBlueprintBasedView(Helper.World, false);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            UMvvmBlueprintLibrary::SetViewModel(View, ViewModel);
            TSharedPtr<SWidget> SWidgetPtr = View->TakeWidget();
        });

        It("Should Support Nested View", [this]
        {
            FTempWorldHelper Helper;

            UClass* WidgetClass = StaticLoadClass(UUserWidget::StaticClass(), nullptr, TEXT("/UnrealMvvmTests/WidgetView/BP_HostWidget.BP_HostWidget_C"));
            UUserWidget* Widget = CreateWidget<UUserWidget>(Helper.World, WidgetClass);

            auto WidgetRef = Widget->TakeWidget();
            UMvvmBlueprintLibrary::SetViewModel(Widget, NewObject<UTestBaseViewModel>());
        });

        It("Should Receive ViewModel via K2Node", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseWidgetViewBlueprint* View = CreateBlueprintBasedView(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            UMvvmBlueprintLibrary::SetViewModel(View, ViewModel);
            TestEqual("NewViewModel", View->ViewModelFromGetter, ViewModel);
        });

        It("Should Get Value From ViewModel via K2Node", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseWidgetViewBlueprint* View = CreateBlueprintBasedView(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            UMvvmBlueprintLibrary::SetViewModel(View, ViewModel);

            ViewModel->SetIntValue(123);

            TestEqual("Value from ViewModel", View->GetValueFromViewModel(), 123);
        });

        It("Should Set Value To ViewModel via K2Node", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseWidgetViewBlueprint* View = CreateBlueprintBasedView(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            UMvvmBlueprintLibrary::SetViewModel(View, ViewModel);

            View->SetValueToViewModel(123);

            TestEqual("Value in ViewModel", ViewModel->GetIntValue(), 123);
        });

        It("Should Set Value To ViewModel via K2Node From Constant", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseWidgetViewBlueprint* View = CreateBlueprintBasedView(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            UMvvmBlueprintLibrary::SetViewModel(View, ViewModel);

            View->SetValueToViewModelConstant();

            TestEqual("Value in ViewModel", ViewModel->GetIntValue(), 123);
        });

        It("Should Set Value To ViewModel via K2Node From Struct", [this]
        {
            FTempWorldHelper Helper;

            UTestBaseWidgetViewBlueprint* View = CreateBlueprintBasedView(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            UMvvmBlueprintLibrary::SetViewModel(View, ViewModel);

            View->SetValueToViewModelStruct();

            TestEqual("Value in ViewModel", ViewModel->GetStructValue().TextData.ToString(), FString(TEXT("123")));
        });

        It("Should Get Value From ViewModel via K2Node Explicit", [this]
        {
            FTempWorldHelper Helper;
            UClass* WidgetClass = StaticLoadClass(UUserWidget::StaticClass(), nullptr, TEXT("/UnrealMvvmTests/WidgetView/BP_TestWidgetView_ExplicitGetSet.BP_TestWidgetView_ExplicitGetSet_C"));

            UTestBaseWidgetViewBlueprint* View = CreateWidget<UTestBaseWidgetViewBlueprint>(Helper.World, WidgetClass);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            UMvvmBlueprintLibrary::SetViewModel(View, ViewModel);

            ViewModel->SetIntValue(123);

            TestEqual("Value from ViewModel", View->GetValueFromViewModel(), 123);
        });

        It("Should Set Value To ViewModel via K2Node Explicit", [this]
        {
            FTempWorldHelper Helper;
            UClass* WidgetClass = StaticLoadClass(UUserWidget::StaticClass(), nullptr, TEXT("/UnrealMvvmTests/WidgetView/BP_TestWidgetView_ExplicitGetSet.BP_TestWidgetView_ExplicitGetSet_C"));

            UTestBaseWidgetViewBlueprint* View = CreateWidget<UTestBaseWidgetViewBlueprint>(Helper.World, WidgetClass);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            UMvvmBlueprintLibrary::SetViewModel(View, ViewModel);

            View->SetValueToViewModel(123);

            TestEqual("Value in ViewModel", ViewModel->GetIntValue(), 123);
        });

        It("Should receive changes when property in path changes", [this]
        {
            FTempWorldHelper Helper;
            UClass* WidgetClass = StaticLoadClass(UUserWidget::StaticClass(), nullptr, TEXT("/UnrealMvvmTests/WidgetView/BP_TestWidgetView_BindingWorker.BP_TestWidgetView_BindingWorker_C"));

            UTestBaseWidgetViewBlueprint* View = CreateWidget<UTestBaseWidgetViewBlueprint>(Helper.World, WidgetClass);
            UBindingWorkerViewModel_Root* RootViewModel = NewObject<UBindingWorkerViewModel_Root>();
            UBindingWorkerViewModel_FirstChild* FirstChildViewModel = NewObject<UBindingWorkerViewModel_FirstChild>();
            UBindingWorkerViewModel_SecondChild* SecondChildViewModel = NewObject<UBindingWorkerViewModel_SecondChild>();

            RootViewModel->SetChild(FirstChildViewModel);
            FirstChildViewModel->SetChild(SecondChildViewModel);

            UMvvmBlueprintLibrary::SetViewModel(View, RootViewModel);
            TSharedPtr<SWidget> SWidgetPtr = View->TakeWidget();

            UBindingWorkerViewModel_SecondChild* SecondChildAlternativeViewModel = NewObject<UBindingWorkerViewModel_SecondChild>();
            SecondChildAlternativeViewModel->SetIntValue(123);
            FirstChildViewModel->SetChild(SecondChildAlternativeViewModel);

            TestEqual("Value from ViewModel", View->MyValue, 123);
        });

        It("Should receive changes in derived blueprint", [this]
        {
            FTempWorldHelper Helper;
            UClass* WidgetClass = StaticLoadClass(UUserWidget::StaticClass(), nullptr, TEXT("/UnrealMvvmTests/WidgetView/BP_TestWidgetView_BlueprintBased_Derived.BP_TestWidgetView_BlueprintBased_Derived_C"));

            UTestBaseWidgetViewBlueprint* View = CreateWidget<UTestBaseWidgetViewBlueprint>(Helper.World, WidgetClass);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            UMvvmBlueprintLibrary::SetViewModel(View, ViewModel);
            TSharedPtr<SWidget> SWidgetPtr = View->TakeWidget();

            ViewModel->SetIntValue(1);
            ViewModel->SetFloatValue(2.f);

            TestEqual("MyValue in View", View->MyValue, 1);
            TestEqual("MyFloatValue in View", View->MyFloatValue, 2.f);
        });

        It("Should receive IsInitial value", [this]
        {
            FTempWorldHelper Helper;
            UClass* WidgetClass = StaticLoadClass(UUserWidget::StaticClass(), nullptr, TEXT("/UnrealMvvmTests/WidgetView/BP_TestWidgetView_IsInitial.BP_TestWidgetView_IsInitial_C"));

            UTestBaseWidgetViewIsInitialTracker* View = CreateWidget<UTestBaseWidgetViewIsInitialTracker>(Helper.World, WidgetClass);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            ViewModel->SetIntValue(1);

            UMvvmBlueprintLibrary::SetViewModel(View, ViewModel);
            TSharedPtr<SWidget> SWidgetPtr = View->TakeWidget();

            ViewModel->SetIntValue(2);

            TestEqual("Num Changes", View->IsInitialResults.Num(), 2);
            TestTrue("Change[0].IsInitial", View->IsInitialResults[0]);
            TestFalse("Change[1].IsInitial", View->IsInitialResults[1]);
        });
    });
}

UTestBaseWidgetViewPureNoBind* FBaseWidgetViewSpec::CreateNativeBasedView(UWorld* World, bool bWithBindings) const
{
    UClass* WidgetClass = bWithBindings ?
        StaticLoadClass(UUserWidget::StaticClass(), nullptr, TEXT("/UnrealMvvmTests/WidgetView/BP_TestWidgetView_NativeBased.BP_TestWidgetView_NativeBased_C")) :
        StaticLoadClass(UUserWidget::StaticClass(), nullptr, TEXT("/UnrealMvvmTests/WidgetView/BP_TestWidgetView_NativeBased_NoBind.BP_TestWidgetView_NativeBased_NoBind_C"));

    return CreateWidget<UTestBaseWidgetViewPureNoBind>(World, WidgetClass);
}

UTestBaseWidgetViewBlueprint* FBaseWidgetViewSpec::CreateBlueprintBasedView(UWorld* World, bool bWithBindings) const
{
    UClass* WidgetClass = bWithBindings ?
        StaticLoadClass(UUserWidget::StaticClass(), nullptr, TEXT("/UnrealMvvmTests/WidgetView/BP_TestWidgetView_BlueprintBased.BP_TestWidgetView_BlueprintBased_C")) :
        StaticLoadClass(UUserWidget::StaticClass(), nullptr, TEXT("/UnrealMvvmTests/WidgetView/BP_TestWidgetView_BlueprintBased_NoBind.BP_TestWidgetView_BlueprintBased_NoBind_C"));

    return CreateWidget<UTestBaseWidgetViewBlueprint>(World, WidgetClass);
}
