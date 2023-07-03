// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Misc/AutomationTest.h"

#include "TestBaseActorView.h"
#include "TestBaseViewModel.h"
#include "Mvvm/MvvmBlueprintLibrary.h"

#include "TempWorldHelper.h"

BEGIN_DEFINE_SPEC(FBaseActorViewSpec, "UnrealMvvm.BaseActorView", EAutomationTestFlags::ClientContext | EAutomationTestFlags::EditorContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)

ATestBaseActorViewBlueprint* CreateBlueprintBasedView(UWorld* World, bool bWithBindings = true) const;

template<typename TActor>
TActor* CreateActor(UWorld* World, UClass* ActorClass = nullptr) const;

END_DEFINE_SPEC(FBaseActorViewSpec)

void FBaseActorViewSpec::Define()
{
    Describe("TBaseView<> derived", [this]
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
    });
}

ATestBaseActorViewBlueprint* FBaseActorViewSpec::CreateBlueprintBasedView(UWorld* World, bool bWithBindings) const
{
    UClass* ActorClass = bWithBindings ?
        StaticLoadClass(AActor::StaticClass(), nullptr, TEXT("/UnrealMvvmTests/BP_TestBaseActorView.BP_TestBaseActorView_C")) :
        StaticLoadClass(AActor::StaticClass(), nullptr, TEXT("/UnrealMvvmTests/BP_TestBaseActorView_NoBind.BP_TestBaseActorView_NoBind_C"));

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

    // emulate what UWorld usually does, after spawnig actor
    // we have to do it manually, because our test Worlds are not fully initialized
    /*Result->PreInitializeComponents();
    Result->InitializeComponents();
    Result->PostInitializeComponents();*/

    return Result;
}