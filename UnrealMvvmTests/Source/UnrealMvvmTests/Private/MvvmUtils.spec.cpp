// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Mvvm/MvvmUtils.h"
#include "UtilsTestViewModel.h"

BEGIN_DEFINE_SPEC(FMvvmUtilsSpec, "UnrealMvvm.Utils", EAutomationTestFlags::ClientContext | EAutomationTestFlags::EditorContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
END_DEFINE_SPEC(FMvvmUtilsSpec)

void FMvvmUtilsSpec::Define()
{
    Describe("SyncViewModelCollection", [this]
    {
        auto DefineTests = [this](auto SyncFunction)
        {
            auto DoTest = [this, SyncFunction](TArray<TObjectPtr<UUtilsTestViewModel>>&& ViewModels)
            {
                TArray<FString> Models{ TEXT("1"), TEXT("2"), TEXT("3") };

                SyncFunction(ViewModels, Models);

                TestEqual("ViewModels.Num()", ViewModels.Num(), Models.Num());
                TestEqual("ViewModels[0]", ViewModels[0]->Model, Models[0]);
                TestEqual("ViewModels[1]", ViewModels[1]->Model, Models[1]);
                TestEqual("ViewModels[2]", ViewModels[2]->Model, Models[2]);
            };

            It("Should Sync to Empty", [this, DoTest]
            {
                DoTest({});
            });

            It("Should Sync to Smaller", [this, DoTest]
            {
                DoTest({ NewObject<UUtilsTestViewModel>(), NewObject<UUtilsTestViewModel>() });
            });

            It("Should Sync to Equal", [this, DoTest]
            {
                DoTest({ NewObject<UUtilsTestViewModel>(), NewObject<UUtilsTestViewModel>(), NewObject<UUtilsTestViewModel>() });
            });

            It("Should Sync to Larger", [this, DoTest]
            {
                DoTest({ NewObject<UUtilsTestViewModel>(), NewObject<UUtilsTestViewModel>(), NewObject<UUtilsTestViewModel>(), NewObject<UUtilsTestViewModel>() });
            });
        };

        auto Factory = []() { return NewObject<UUtilsTestViewModel>(); };
        auto Setter = [](auto* ViewModel, auto& Model) { ViewModel->SetModel(Model); };

        Describe("Default Factory, Default Setter", [this, DefineTests]
        {
            Describe("RawPtr", [this, DefineTests]
            {
                DefineTests([](TArray<UUtilsTestViewModel*>& ViewModels, auto& Models)
                {
                    MvvmUtils::SyncViewModelCollection(ViewModels, Models);
                });
            });

            Describe("ObjectPtr", [this, DefineTests]
            {
                DefineTests([](TArray<TObjectPtr<UUtilsTestViewModel>>& ViewModels, auto& Models)
                {
                    MvvmUtils::SyncViewModelCollection(ViewModels, Models);
                });
            });
        });

        Describe("Custom Factory, Default Setter", [this, DefineTests, Factory]
        {
            Describe("RawPtr", [this, DefineTests, Factory]
            {
                DefineTests([Factory](TArray<UUtilsTestViewModel*>& ViewModels, auto& Models)
                {
                    MvvmUtils::SyncViewModelCollection(ViewModels, Models, Factory);
                });
            });

            Describe("ObjectPtr", [this, DefineTests, Factory]
            {
                DefineTests([Factory](TArray<TObjectPtr<UUtilsTestViewModel>>& ViewModels, auto& Models)
                {
                    MvvmUtils::SyncViewModelCollection(ViewModels, Models, Factory);
                });
            });
        });

        Describe("Default Factory, Custom Setter", [this, DefineTests, Setter]
        {
            Describe("RawPtr", [this, DefineTests, Setter]
            {
                DefineTests([Setter](TArray<UUtilsTestViewModel*>& ViewModels, auto& Models)
                {
                    MvvmUtils::SyncViewModelCollection(ViewModels, Models, Setter);
                });
            });

            Describe("ObjectPtr", [this, DefineTests, Setter]
            {
                DefineTests([Setter](TArray<TObjectPtr<UUtilsTestViewModel>>& ViewModels, auto& Models)
                {
                    MvvmUtils::SyncViewModelCollection(ViewModels, Models, Setter);
                });
            });
        });

        Describe("Custom Factory, Custom Setter", [this, DefineTests, Factory, Setter]
        {
            Describe("RawPtr", [this, DefineTests, Factory, Setter]
            {
                DefineTests([Factory, Setter](TArray<UUtilsTestViewModel*>& ViewModels, auto& Models)
                {
                    MvvmUtils::SyncViewModelCollection(ViewModels, Models, Factory, Setter);
                });
            });

            Describe("ObjectPtr", [this, DefineTests, Factory, Setter]
            {
                DefineTests([Factory, Setter](TArray<TObjectPtr<UUtilsTestViewModel>>& ViewModels, auto& Models)
                {
                    MvvmUtils::SyncViewModelCollection(ViewModels, Models, Factory, Setter);
                });
            });
        });

        Describe("From Map", [this]
        {
            It("Should Sync from TMap", [this]
            {
                TMap<int32, FString> Models{ {0, TEXT("1") }, {1, TEXT("2")}, {2, TEXT("3")} };
                TArray<UUtilsTestViewModel*> ViewModels;
                MvvmUtils::SyncViewModelCollection(ViewModels, Models, [](UUtilsTestViewModel* VM, const TPair<int32, FString>& Model)
                {
                    VM->SetModel(Model.Value);
                });

                TestEqual("ViewModels.Num()", ViewModels.Num(), Models.Num());
                TestEqual("ViewModels[0]", ViewModels[0]->Model, Models[0]);
                TestEqual("ViewModels[1]", ViewModels[1]->Model, Models[1]);
                TestEqual("ViewModels[2]", ViewModels[2]->Model, Models[2]);
            });
        });
    });
}
