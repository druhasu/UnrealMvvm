// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Misc/AutomationTest.h"

#include "TestBaseActorView.h"
#include "TestBaseViewModel.h"
#include "BindingWorkerTestViewModel.h"
#include "Mvvm/MvvmBlueprintLibrary.h"

#include "TempWorldHelper.h"

BEGIN_DEFINE_SPEC(FBaseActorViewSpec, "UnrealMvvm.BaseActorView", EAutomationTestFlags::ClientContext | EAutomationTestFlags::EditorContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)

ATestBaseActorViewPureNoBind* CreateNativeBasedView(UWorld* World, bool bWithBindings = true) const;
ATestBaseActorViewBlueprint* CreateBlueprintBasedView(UWorld* World, bool bWithBindings = true) const;

template<typename TActor>
TActor* CreateActor(UWorld* World, UClass* ActorClass = nullptr) const;

END_DEFINE_SPEC(FBaseActorViewSpec)

void FBaseActorViewSpec::Define()
{
    Describe("TBaseView<> derived Native", [this]
    {
        It("Should Construct Actor", [this]
        {
            // this test just checks our assumptions about how Actor Views are constructed and destroyed
            FTempWorldHelper Helper;

            ATestBaseActorViewPure* View = CreateActor<ATestBaseActorViewPure>(Helper.World);
            TestNotNull("View", View);

            TestTrue("View is Initialized", View->IsActorInitialized());
            TestFalse("View HasBegunPlay", View->HasActorBegunPlay());

            View->DispatchBeginPlay();
            TestTrue("View HasBegunPlay", View->HasActorBegunPlay());

            View->RouteEndPlay(EEndPlayReason::EndPlayInEditor);
            TestFalse("View HasBegunPlay", View->HasActorBegunPlay());
        });
        
        It("Should Receive Changes When Constructed And Then Set ViewModel", [this]
        {
            FTempWorldHelper Helper;

            ATestBaseActorViewPure* View = CreateActor<ATestBaseActorViewPure>(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            ViewModel->SetIntValue(1);

            TestEqual("MyValue is default", View->MyValue, 0);

            View->DispatchBeginPlay();
            View->SetViewModel(ViewModel);
            TestEqual("MyValue", View->MyValue, 1);

            ViewModel->SetIntValue(2);
            TestEqual("MyValue", View->MyValue, 2);
        });

        It("Should Receive Changes When Set ViewModel And Then Constructed", [this]
        {
            FTempWorldHelper Helper;

            ATestBaseActorViewPure* View = CreateActor<ATestBaseActorViewPure>(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            ViewModel->SetIntValue(1);

            TestEqual("MyValue is default", View->MyValue, 0);

            View->SetViewModel(ViewModel);
            View->DispatchBeginPlay();
            TestEqual("MyValue is default", View->MyValue, 1);

            ViewModel->SetIntValue(2);
            TestEqual("MyValue is updated", View->MyValue, 2);
        });

        It("Should Not Receive Changes When Only Constructed", [this]
        {
            FTempWorldHelper Helper;

            ATestBaseActorViewPure* View = CreateActor<ATestBaseActorViewPure>(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            View->DispatchBeginPlay();

            TestEqual("MyValue is default", View->MyValue, 0);

            ViewModel->SetIntValue(1);
            TestEqual("MyValue is default", View->MyValue, 0);
        });

        It("Should Not Receive Changes When Only Set ViewModel", [this]
        {
            FTempWorldHelper Helper;

            ATestBaseActorViewPure* View = CreateActor<ATestBaseActorViewPure>(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            View->SetViewModel(ViewModel);

            TestEqual("MyValue is default", View->MyValue, 0);

            ViewModel->SetIntValue(1);
            TestEqual("MyValue is default", View->MyValue, 0);
        });

        It("Should Not Receive Changes After Destructed", [this]
        {
            FTempWorldHelper Helper;

            ATestBaseActorViewPure* View = CreateActor<ATestBaseActorViewPure>(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            View->DispatchBeginPlay();
            View->SetViewModel(ViewModel);

            ViewModel->SetIntValue(1);

            View->RouteEndPlay(EEndPlayReason::EndPlayInEditor);
            ViewModel->SetIntValue(2);
            TestEqual("MyValue", View->MyValue, 1);
        });

        It("Should Not Receive Changes After Unset ViewModel", [this]
        {
            FTempWorldHelper Helper;

            ATestBaseActorViewPure* View = CreateActor<ATestBaseActorViewPure>(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            View->DispatchBeginPlay();
            View->SetViewModel(ViewModel);

            ViewModel->SetIntValue(1);

            View->SetViewModel(nullptr);
            ViewModel->SetIntValue(2);
            TestEqual("MyValue", View->MyValue, 1);
        });

        It("Should Return Untyped ViewModel", [this]
        {
            FTempWorldHelper Helper;

            ATestBaseActorViewPure* View = CreateActor<ATestBaseActorViewPure>(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            View->SetViewModel(ViewModel);

            TestEqual("Untyped ViewModel", UMvvmBlueprintLibrary::GetViewModel(View), StaticCast<UBaseViewModel*>(ViewModel));
        });

        It("Should Call OnViewModelChanged", [this]
        {
            FTempWorldHelper Helper;

            ATestBaseActorViewPure* View = CreateActor<ATestBaseActorViewPure>(Helper.World);
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

            ATestBaseActorViewPureNoBind* View = CreateActor<ATestBaseActorViewPureNoBind>(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            View->SetViewModel(ViewModel);
            View->DispatchBeginPlay();
        });

        It("Should report initialization when Constructed and then Set ViewModel", [this]
        {
            FTempWorldHelper Helper;

            ATestBaseActorViewPure* View = CreateActor<ATestBaseActorViewPure>(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            ViewModel->SetIntValue(1);

            View->IntValueChangedCallback = [&](auto)
            {
                TestTrue("IsInitializing by View", UMvvmBlueprintLibrary::IsInitializingProperty(View));
                TestTrue("IsInitializing by ViewModel", UMvvmBlueprintLibrary::IsInitializingProperty(ViewModel));
            };

            TestFalse("Is Initializing by View", UMvvmBlueprintLibrary::IsInitializingProperty(View));
            TestFalse("Is Initializing by ViewModel", UMvvmBlueprintLibrary::IsInitializingProperty(ViewModel));

            View->DispatchBeginPlay();
            View->SetViewModel(ViewModel);

            TestFalse("Is Initializing by View", UMvvmBlueprintLibrary::IsInitializingProperty(View));
            TestFalse("Is Initializing by ViewModel", UMvvmBlueprintLibrary::IsInitializingProperty(ViewModel));
        });

        It("Should report initialization when Set ViewModel and then Constructed", [this]
        {
            FTempWorldHelper Helper;

            ATestBaseActorViewPure* View = CreateActor<ATestBaseActorViewPure>(Helper.World);
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
            View->DispatchBeginPlay();

            TestFalse("Is Initializing by View", UMvvmBlueprintLibrary::IsInitializingProperty(View));
            TestFalse("Is Initializing by ViewModel", UMvvmBlueprintLibrary::IsInitializingProperty(ViewModel));
        });

        It("Should report change", [this]
        {
            FTempWorldHelper Helper;

            ATestBaseActorViewPure* View = CreateActor<ATestBaseActorViewPure>(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            ViewModel->SetIntValue(1);

            View->DispatchBeginPlay();
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

            ATestBaseActorViewPure* View = CreateActor<ATestBaseActorViewPure>(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            ViewModel->SetIntValue(1);

            View->DispatchBeginPlay();
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

            ATestBaseActorViewPure* View = CreateActor<ATestBaseActorViewPure>(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            ViewModel->SetIntValue(1);

            View->IntValueChangedCallback = [&](auto)
            {
                TestFalse("IsChanging by View", UMvvmBlueprintLibrary::IsChangingProperty(View));
                TestFalse("IsChanging by ViewModel", UMvvmBlueprintLibrary::IsChangingProperty(ViewModel));
            };

            View->SetViewModel(ViewModel);
            View->DispatchBeginPlay();
        });

        It("Should not report change during initialization when Constructed and then Set ViewModel", [this]
        {
            FTempWorldHelper Helper;

            ATestBaseActorViewPure* View = CreateActor<ATestBaseActorViewPure>(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            ViewModel->SetIntValue(1);

            View->IntValueChangedCallback = [&](auto)
            {
                TestFalse("IsChanging by View", UMvvmBlueprintLibrary::IsChangingProperty(View));
                TestFalse("IsChanging by ViewModel", UMvvmBlueprintLibrary::IsChangingProperty(ViewModel));
            };

            View->DispatchBeginPlay();
            View->SetViewModel(ViewModel);
        });
    });

    Describe("TBaseView<> derived in Blueprint", [this]
    {
        It("Should Construct Actor", [this]
        {
            // this test just checks our assumptions about how Actor Views are constructed and destroyed
            FTempWorldHelper Helper;

            ATestBaseActorViewPureNoBind* View = CreateNativeBasedView(Helper.World);
            TestNotNull("View", View);

            TestTrue("View is Initialized", View->IsActorInitialized());
            TestFalse("View HasBegunPlay", View->HasActorBegunPlay());

            View->DispatchBeginPlay();
            TestTrue("View HasBegunPlay", View->HasActorBegunPlay());

            View->RouteEndPlay(EEndPlayReason::EndPlayInEditor);
            TestFalse("View HasBegunPlay", View->HasActorBegunPlay());
        });

        It("Should Receive Changes When Constructed And Then Set ViewModel", [this]
        {
            FTempWorldHelper Helper;

            ATestBaseActorViewPureNoBind* View = CreateNativeBasedView(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            ViewModel->SetIntValue(1);

            TestEqual("MyValue is default", View->MyValue, 0);

            View->DispatchBeginPlay();
            View->SetViewModel(ViewModel);
            TestEqual("MyValue", View->MyValue, 1);

            ViewModel->SetIntValue(2);
            TestEqual("MyValue", View->MyValue, 2);
        });

        It("Should Receive Changes When Set ViewModel And Then Constructed", [this]
        {
            FTempWorldHelper Helper;

            ATestBaseActorViewPureNoBind* View = CreateNativeBasedView(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            ViewModel->SetIntValue(1);

            TestEqual("MyValue is default", View->MyValue, 0);

            View->SetViewModel(ViewModel);
            View->DispatchBeginPlay();
            TestEqual("MyValue is default", View->MyValue, 1);

            ViewModel->SetIntValue(2);
            TestEqual("MyValue is updated", View->MyValue, 2);
        });

        It("Should Not Receive Changes When Only Constructed", [this]
        {
            FTempWorldHelper Helper;

            ATestBaseActorViewPureNoBind* View = CreateNativeBasedView(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            View->DispatchBeginPlay();

            TestEqual("MyValue is default", View->MyValue, 0);

            ViewModel->SetIntValue(1);
            TestEqual("MyValue is default", View->MyValue, 0);
        });

        It("Should Not Receive Changes When Only Set ViewModel", [this]
        {
            FTempWorldHelper Helper;

            ATestBaseActorViewPureNoBind* View = CreateNativeBasedView(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            View->SetViewModel(ViewModel);

            TestEqual("MyValue is default", View->MyValue, 0);

            ViewModel->SetIntValue(1);
            TestEqual("MyValue is default", View->MyValue, 0);
        });

        It("Should Not Receive Changes After Destructed", [this]
        {
            FTempWorldHelper Helper;

            ATestBaseActorViewPureNoBind* View = CreateNativeBasedView(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            View->DispatchBeginPlay();
            View->SetViewModel(ViewModel);

            ViewModel->SetIntValue(1);

            View->RouteEndPlay(EEndPlayReason::EndPlayInEditor);
            ViewModel->SetIntValue(2);
            TestEqual("MyValue", View->MyValue, 1);
        });

        It("Should Not Receive Changes After Unset ViewModel", [this]
        {
            FTempWorldHelper Helper;

            ATestBaseActorViewPureNoBind* View = CreateNativeBasedView(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            View->DispatchBeginPlay();
            View->SetViewModel(ViewModel);

            ViewModel->SetIntValue(1);

            View->SetViewModel(nullptr);
            ViewModel->SetIntValue(2);
            TestEqual("MyValue", View->MyValue, 1);
        });

        It("Should Return Untyped ViewModel", [this]
        {
            FTempWorldHelper Helper;

            ATestBaseActorViewPureNoBind* View = CreateNativeBasedView(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            View->SetViewModel(ViewModel);

            TestEqual("Untyped ViewModel", UMvvmBlueprintLibrary::GetViewModel(View), StaticCast<UBaseViewModel*>(ViewModel));
        });

        It("Should Call OnViewModelChanged", [this]
        {
            FTempWorldHelper Helper;

            ATestBaseActorViewPureNoBind* View = CreateNativeBasedView(Helper.World);
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

            ATestBaseActorViewPureNoBind* View = CreateNativeBasedView(Helper.World, false);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            View->SetViewModel(ViewModel);
            View->DispatchBeginPlay();
        });
    });

    Describe("Blueprint only", [this]
    {
        It("Should Construct Actor", [this]
        {
            // this test just checks our assumptions about how Actor Views are constructed and destroyed
            FTempWorldHelper Helper;

            ATestBaseActorViewBlueprint* View = CreateBlueprintBasedView(Helper.World);
            TestNotNull("View", View);

            TestTrue("View is Initialized", View->IsActorInitialized());
            TestFalse("View HasBegunPlay", View->HasActorBegunPlay());

            View->DispatchBeginPlay();
            TestTrue("View HasBegunPlay", View->HasActorBegunPlay());

            View->RouteEndPlay(EEndPlayReason::EndPlayInEditor);
            TestFalse("View HasBegunPlay", View->HasActorBegunPlay());
        });

        It("Should Receive Changes When Constructed And Then Set ViewModel", [this]
        {
            FTempWorldHelper Helper;

            ATestBaseActorViewBlueprint* View = CreateBlueprintBasedView(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            ViewModel->SetIntValue(1);

            TestEqual("MyValue is default", View->MyValue, 0);

            View->DispatchBeginPlay();
            UMvvmBlueprintLibrary::SetViewModel(View, ViewModel);
            TestEqual("MyValue", View->MyValue, 1);

            ViewModel->SetIntValue(2);
            TestEqual("MyValue", View->MyValue, 2);
        });

        It("Should Receive Changes When Set ViewModel And Then Constructed", [this]
        {
            FTempWorldHelper Helper;

            ATestBaseActorViewBlueprint* View = CreateBlueprintBasedView(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            ViewModel->SetIntValue(1);

            TestEqual("MyValue is default", View->MyValue, 0);

            UMvvmBlueprintLibrary::SetViewModel(View, ViewModel);
            View->DispatchBeginPlay();
            TestEqual("MyValue is default", View->MyValue, 1);

            ViewModel->SetIntValue(2);
            TestEqual("MyValue is updated", View->MyValue, 2);
        });

        It("Should Not Receive Changes When Only Constructed", [this]
        {
            FTempWorldHelper Helper;

            ATestBaseActorViewBlueprint* View = CreateBlueprintBasedView(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            View->DispatchBeginPlay();

            TestEqual("MyValue is default", View->MyValue, 0);

            ViewModel->SetIntValue(1);
            TestEqual("MyValue is default", View->MyValue, 0);
        });

        It("Should Not Receive Changes When Only Set ViewModel", [this]
        {
            FTempWorldHelper Helper;

            ATestBaseActorViewBlueprint* View = CreateBlueprintBasedView(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            UMvvmBlueprintLibrary::SetViewModel(View, ViewModel);

            TestEqual("MyValue is default", View->MyValue, 0);

            ViewModel->SetIntValue(1);
            TestEqual("MyValue is default", View->MyValue, 0);
        });

        It("Should Not Receive Changes After Destructed", [this]
        {
            FTempWorldHelper Helper;

            ATestBaseActorViewBlueprint* View = CreateBlueprintBasedView(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            View->DispatchBeginPlay();
            UMvvmBlueprintLibrary::SetViewModel(View, ViewModel);

            ViewModel->SetIntValue(1);

            View->RouteEndPlay(EEndPlayReason::EndPlayInEditor);
            ViewModel->SetIntValue(2);
            TestEqual("MyValue", View->MyValue, 1);
        });

        It("Should Not Receive Changes After Unset ViewModel", [this]
        {
            FTempWorldHelper Helper;

            ATestBaseActorViewBlueprint* View = CreateBlueprintBasedView(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            View->DispatchBeginPlay();
            UMvvmBlueprintLibrary::SetViewModel(View, ViewModel);

            ViewModel->SetIntValue(1);

            UMvvmBlueprintLibrary::SetViewModel(View, nullptr);
            ViewModel->SetIntValue(2);
            TestEqual("MyValue", View->MyValue, 1);
        });

        It("Should Return Untyped ViewModel", [this]
        {
            FTempWorldHelper Helper;

            ATestBaseActorViewBlueprint* View = CreateBlueprintBasedView(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            UMvvmBlueprintLibrary::SetViewModel(View, ViewModel);

            TestEqual("Untyped ViewModel", UMvvmBlueprintLibrary::GetViewModel(View), StaticCast<UBaseViewModel*>(ViewModel));
        });

        It("Should Call OnViewModelChanged", [this]
        {
            FTempWorldHelper Helper;

            ATestBaseActorViewBlueprint* View = CreateBlueprintBasedView(Helper.World);
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

            ATestBaseActorViewBlueprint* View = CreateBlueprintBasedView(Helper.World, false);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            UMvvmBlueprintLibrary::SetViewModel(View, ViewModel);
            View->DispatchBeginPlay();
        });

        It("Should Receive ViewModel via K2Node", [this]
        {
            FTempWorldHelper Helper;

            ATestBaseActorViewBlueprint* View = CreateBlueprintBasedView(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            UMvvmBlueprintLibrary::SetViewModel(View, ViewModel);
            TestEqual("NewViewModel", View->ViewModelFromGetter, ViewModel);
        });

        It("Should Get Value From ViewModel via K2Node", [this]
        {
            FTempWorldHelper Helper;

            ATestBaseActorViewBlueprint* View = CreateBlueprintBasedView(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            UMvvmBlueprintLibrary::SetViewModel(View, ViewModel);

            ViewModel->SetIntValue(123);

            TestEqual("Value from ViewModel", View->GetValueFromViewModel(), 123);
        });

        It("Should Set Value To ViewModel via K2Node", [this]
        {
            FTempWorldHelper Helper;

            ATestBaseActorViewBlueprint* View = CreateBlueprintBasedView(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            UMvvmBlueprintLibrary::SetViewModel(View, ViewModel);

            View->SetValueToViewModel(123);

            TestEqual("Value in ViewModel", ViewModel->GetIntValue(), 123);
        });

        It("Should Set Value To ViewModel via K2Node From Constant", [this]
        {
            FTempWorldHelper Helper;

            ATestBaseActorViewBlueprint* View = CreateBlueprintBasedView(Helper.World);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            UMvvmBlueprintLibrary::SetViewModel(View, ViewModel);

            View->SetValueToViewModelConstant();

            TestEqual("Value in ViewModel", ViewModel->GetIntValue(), 123);
        });

        It("Should Get Value From ViewModel via K2Node Explicit", [this]
        {
            FTempWorldHelper Helper;
            UClass* ActorClass = StaticLoadClass(AActor::StaticClass(), nullptr, TEXT("/UnrealMvvmTests/ActorView/BP_TestActorView_ExplicitGetSet.BP_TestActorView_ExplicitGetSet_C"));

            ATestBaseActorViewBlueprint* View = CreateActor<ATestBaseActorViewBlueprint>(Helper.World, ActorClass);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            UMvvmBlueprintLibrary::SetViewModel(View, ViewModel);

            ViewModel->SetIntValue(123);

            TestEqual("Value from ViewModel", View->GetValueFromViewModel(), 123);
        });

        It("Should Set Value To ViewModel via K2Node Explicit", [this]
        {
            FTempWorldHelper Helper;
            UClass* ActorClass = StaticLoadClass(AActor::StaticClass(), nullptr, TEXT("/UnrealMvvmTests/ActorView/BP_TestActorView_ExplicitGetSet.BP_TestActorView_ExplicitGetSet_C"));

            ATestBaseActorViewBlueprint* View = CreateActor<ATestBaseActorViewBlueprint>(Helper.World, ActorClass);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            UMvvmBlueprintLibrary::SetViewModel(View, ViewModel);

            View->SetValueToViewModel(123);

            TestEqual("Value in ViewModel", ViewModel->GetIntValue(), 123);
        });

        It("Should receive changes when property in path changes", [this]
        {
            FTempWorldHelper Helper;
            UClass* ActorClass = StaticLoadClass(AActor::StaticClass(), nullptr, TEXT("/UnrealMvvmTests/ActorView/BP_TestActorView_BindingWorker.BP_TestActorView_BindingWorker_C"));

            ATestBaseActorViewBlueprint* View = CreateActor<ATestBaseActorViewBlueprint>(Helper.World, ActorClass);
            UBindingWorkerViewModel_Root* RootViewModel = NewObject<UBindingWorkerViewModel_Root>();
            UBindingWorkerViewModel_FirstChild* FirstChildViewModel = NewObject<UBindingWorkerViewModel_FirstChild>();
            UBindingWorkerViewModel_SecondChild* SecondChildViewModel = NewObject<UBindingWorkerViewModel_SecondChild>();

            RootViewModel->SetChild(FirstChildViewModel);
            FirstChildViewModel->SetChild(SecondChildViewModel);

            UMvvmBlueprintLibrary::SetViewModel(View, RootViewModel);
            View->DispatchBeginPlay();

            UBindingWorkerViewModel_SecondChild* SecondChildAlternativeViewModel = NewObject<UBindingWorkerViewModel_SecondChild>();
            SecondChildAlternativeViewModel->SetIntValue(123);
            FirstChildViewModel->SetChild(SecondChildAlternativeViewModel);

            TestEqual("Value from ViewModel", View->MyValue, 123);
        });

        It("Should receive changes in derived blueprint", [this]
        {
            FTempWorldHelper Helper;
            UClass* ActorClass = StaticLoadClass(AActor::StaticClass(), nullptr, TEXT("/UnrealMvvmTests/ActorView/BP_TestActorView_BlueprintBased_Derived.BP_TestActorView_BlueprintBased_Derived_C"));

            ATestBaseActorViewBlueprint* View = CreateActor<ATestBaseActorViewBlueprint>(Helper.World, ActorClass);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();

            UMvvmBlueprintLibrary::SetViewModel(View, ViewModel);
            View->DispatchBeginPlay();

            ViewModel->SetIntValue(1);
            ViewModel->SetFloatValue(2.f);

            TestEqual("MyValue in View", View->MyValue, 1);
            TestEqual("MyFloatValue in View", View->MyFloatValue, 2.f);
        });

        It("Should receive IsInitial value", [this]
        {
            FTempWorldHelper Helper;
            UClass* WidgetClass = StaticLoadClass(AActor::StaticClass(), nullptr, TEXT("/UnrealMvvmTests/ActorView/BP_TestActorView_IsInitial.BP_TestActorView_IsInitial_C"));

            ATestBaseActorViewIsInitialTracker* View = CreateActor<ATestBaseActorViewIsInitialTracker>(Helper.World, WidgetClass);
            UTestBaseViewModel* ViewModel = NewObject<UTestBaseViewModel>();
            ViewModel->SetIntValue(1);

            UMvvmBlueprintLibrary::SetViewModel(View, ViewModel);
            View->DispatchBeginPlay();

            ViewModel->SetIntValue(2);

            TestEqual("Num Changes", View->IsInitialResults.Num(), 2);
            TestTrue("Change[0].IsInitial", View->IsInitialResults[0]);
            TestFalse("Change[1].IsInitial", View->IsInitialResults[1]);
        });
    });
}

ATestBaseActorViewPureNoBind* FBaseActorViewSpec::CreateNativeBasedView(UWorld* World, bool bWithBindings) const
{
    UClass* ActorClass = bWithBindings ?
        StaticLoadClass(AActor::StaticClass(), nullptr, TEXT("/UnrealMvvmTests/ActorView/BP_TestActorView_NativeBased.BP_TestActorView_NativeBased_C")) :
        StaticLoadClass(AActor::StaticClass(), nullptr, TEXT("/UnrealMvvmTests/ActorView/BP_TestActorView_NativeBased_NoBind.BP_TestActorView_NativeBased_NoBind_C"));

    return CreateActor<ATestBaseActorViewPureNoBind>(World, ActorClass);
}

ATestBaseActorViewBlueprint* FBaseActorViewSpec::CreateBlueprintBasedView(UWorld* World, bool bWithBindings) const
{
    UClass* ActorClass = bWithBindings ?
        StaticLoadClass(AActor::StaticClass(), nullptr, TEXT("/UnrealMvvmTests/ActorView/BP_TestActorView_BlueprintBased.BP_TestActorView_BlueprintBased_C")) :
        StaticLoadClass(AActor::StaticClass(), nullptr, TEXT("/UnrealMvvmTests/ActorView/BP_TestActorView_BlueprintBased_NoBind.BP_TestActorView_BlueprintBased_NoBind_C"));

    return CreateActor<ATestBaseActorViewBlueprint>(World, ActorClass);
}

template<typename TActor>
TActor* FBaseActorViewSpec::CreateActor(UWorld* World, UClass* ActorClass) const
{
    if (ActorClass == nullptr)
    {
        ActorClass = TActor::StaticClass();
    }

    TActor* Result = World->SpawnActor<TActor>(ActorClass);

    return Result;
}
