// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Modules/ModuleManager.h"
#include "Mvvm/Impl/BaseView/ViewRegistry.h"
#include "Mvvm/Impl/BaseView/WidgetExtensionsAccessor.h"
#include "Slate/ViewModelPropertiesSummoner.h"
#include "Slate/UnrealMvvmEditorStyle.h"
#include "BaseViewBlueprintExtension.h"
#include "ViewModelClassSelectorHelper.h"
#include "UMGEditorModule.h"
#include "BlueprintEditorModule.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "BlueprintModes/WidgetBlueprintApplicationMode.h"
#include "BlueprintModes/WidgetBlueprintApplicationModes.h"
#include "WidgetBlueprint.h"
#include "Blueprint/WidgetTree.h"
#include "KismetCompiler.h"

class FUnrealMvvmEditorModule;

struct FBlueprintCreateListener : public FUObjectArray::FUObjectCreateListener
{
    FBlueprintCreateListener(FUnrealMvvmEditorModule* InModule);
    ~FBlueprintCreateListener();

    void NotifyUObjectCreated(const class UObjectBase* Object, int32 Index) override;
    void OnUObjectArrayShutdown() override;

    FUnrealMvvmEditorModule* Module;
};

class FUnrealMvvmEditorModule : public IModuleInterface
{
public:
    friend struct FBlueprintCreateListener;

    using ThisClass = FUnrealMvvmEditorModule;

    void StartupModule() override
    {
        WithModule<IUMGEditorModule>("UMGEditor", [&](IUMGEditorModule& UMGEditorModule)
        {
            UMGEditorModule.OnRegisterTabsForEditor().AddRaw(this, &ThisClass::RegisterWidgetBlueprintEditorTab);
        });

        WithModule<FBlueprintEditorModule>("Kismet", [&](FBlueprintEditorModule& BlueprintEditorModule)
        {
            BlueprintEditorModule.OnRegisterTabsForEditor().AddRaw(this, &ThisClass::RegisterActorBlueprintEditorTab);
            BlueprintEditorModule.OnRegisterLayoutExtensions().AddRaw(this, &ThisClass::RegisterActorBlueprintEditorLayoutExtension);
        });

        FCoreUObjectDelegates::OnObjectPreSave.AddRaw(this, &ThisClass::OnObjectPreSave);

        if (GEditor != nullptr)
        {
            OnPostEngineInit();
        }
        else
        {
            FCoreDelegates::OnPostEngineInit.AddRaw(this, &ThisClass::OnPostEngineInit);
        }
    }

    void ShutdownModule() override
    {
        FUnrealMvvmEditorStyle::Shutdown();

        BlueprintListener = nullptr;

        if (IUMGEditorModule* UMGEditorModule = FModuleManager::GetModulePtr<IUMGEditorModule>("UMGEditor"))
        {
            UMGEditorModule->OnRegisterTabsForEditor().RemoveAll(this);
        }

        if (FBlueprintEditorModule* BlueprintEditorModule = FModuleManager::GetModulePtr<FBlueprintEditorModule>("Kismet"))
        {
            BlueprintEditorModule->OnRegisterTabsForEditor().RemoveAll(this);
            BlueprintEditorModule->OnRegisterLayoutExtensions().RemoveAll(this);
        }

        FCoreUObjectDelegates::OnObjectPreSave.RemoveAll(this);

        if (GEditor)
        {
            GEditor->OnBlueprintPreCompile().RemoveAll(this);
        }
    }

private:
    template <typename TModule>
    void WithModule(FName ModuleName, TFunction<void(TModule& Module)> Action)
    {
        if (FModuleManager::Get().IsModuleLoaded(ModuleName))
        {
            Action(FModuleManager::GetModuleChecked<TModule>(ModuleName));
        }
        else
        {
            TSharedRef<FDelegateHandle> Handle = MakeShared<FDelegateHandle>();
            *Handle = FModuleManager::Get().OnModulesChanged().AddLambda([=](FName NewModuleName, EModuleChangeReason ChangeReason)
            {
                if (ChangeReason == EModuleChangeReason::ModuleLoaded && NewModuleName == ModuleName)
                {
                    Action(FModuleManager::GetModuleChecked<TModule>(ModuleName));
                    FModuleManager::Get().OnModulesChanged().Remove(*Handle);
                }
            });
        }
    }

    void RegisterWidgetBlueprintEditorTab(const FWidgetBlueprintApplicationMode& ApplicationMode, FWorkflowAllowedTabSet& TabFactories)
    {
        if (ApplicationMode.GetModeName() == FWidgetBlueprintApplicationModes::DesignerMode ||
            ApplicationMode.GetModeName() == FWidgetBlueprintApplicationModes::GraphMode)
        {
            TabFactories.RegisterFactory(MakeShared<FViewModelPropertiesSummoner>(ApplicationMode.GetBlueprintEditor()));

            if (ApplicationMode.LayoutExtender)
            {
                static const FName DetailsTabID("WidgetDetails");

                FTabManager::FTab NewTab(FTabId(FViewModelPropertiesSummoner::TabID, ETabIdFlags::SaveLayout), ETabState::ClosedTab);
                ApplicationMode.LayoutExtender->ExtendLayout(DetailsTabID, ELayoutExtensionPosition::After, NewTab);
            }
        }
    }

    void RegisterActorBlueprintEditorTab(FWorkflowAllowedTabSet& TabFactories, FName ModeName, TSharedPtr<FBlueprintEditor> Editor)
    {
        if (Editor)
        {
            if (TSubclassOf<UObject> ParentClass = Editor->GetBlueprintObj()->ParentClass)
            {
                if (ParentClass->IsChildOf<AActor>())
                {
                    TabFactories.RegisterFactory(MakeShared<FViewModelPropertiesSummoner>(Editor));
                }
            }
        }
    }

    void RegisterActorBlueprintEditorLayoutExtension(FLayoutExtender& LayoutExtender)
    {
        static const FName DetailsTabID("Inspector");

        FTabManager::FTab NewTab(FTabId(FViewModelPropertiesSummoner::TabID, ETabIdFlags::SaveLayout), ETabState::ClosedTab);
        LayoutExtender.ExtendLayout(DetailsTabID, ELayoutExtensionPosition::After, NewTab);
    }

    void OnObjectPreSave(UObject* Asset, FObjectPreSaveContext Context)
    {
        UBlueprint* SavedBlueprint = Cast<UBlueprint>(Asset);
        if (!SavedBlueprint)
        {
            // it's not a Blueprint, ignore
            return;
        }

        UClass* NativeViewClass = FBlueprintEditorUtils::GetNativeParent(SavedBlueprint);

        if (NativeViewClass == nullptr)
        {
            // this Blueprint was saved with base class that no longer exist, ignore it
            return;
        }

        if (UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(SavedBlueprint))
        {
            FixupWidgetExtensions(WidgetBlueprint);
        }

        UBaseViewBlueprintExtension::TryRemoveUnnecessary(SavedBlueprint);
    }

    void FixupWidgetExtensions(UWidgetBlueprint* SavedBlueprint)
    {
        using namespace UnrealMvvm_Impl;

        // fixup corrupted Extensions from previous versions
        // Extensions property is not marked Transient, so it will save all Extensions created during edit time
        // but they won't load properly, leaving nullptrs instead
        // here we remove all nullptrs from Extensions array before saving this widget blueprint
        SavedBlueprint->WidgetTree->ForEachWidget([](UWidget* TemplateWidget)
        {
            if (UUserWidget* TemplateUserWidget = Cast<UUserWidget>(TemplateWidget))
            {
                TArray<TObjectPtr<UUserWidgetExtension>>& Extensions = TemplateUserWidget->*GetPrivate(ExtensionsAccessor());
                Extensions.RemoveSwap(nullptr);
            }
        });
    }

    void OnPostEngineInit()
    {
        if (GEditor != nullptr)
        {
            GEditor->OnBlueprintPreCompile().AddRaw(this, &ThisClass::OnBlueprintPreCompile);
        }
        else
        {
            // we hit this path when launching uncooked game
            // in this case WITH_EDITOR is defined, but GEditor is nullptr
            // we still need to be able to patch Blueprints containing View classes

            // listen to creation of UObjects and collect all UBlueprint instances into PendingBlueprints
            BlueprintListener = MakeUnique<FBlueprintCreateListener>(this);
            GUObjectArray.AddUObjectCreateListener(BlueprintListener.Get());

            // listen to event right before UBlueprint is compiled and add our UBaseViewExtension into it
            // this event does not tell us which UBlueprint is compiled, so we have check them all
            FKismetCompilerContext::OnPreCompile.AddRaw(this, &ThisClass::OnKismetPreCompile);
        }

        FCoreDelegates::OnPostEngineInit.RemoveAll(this);
    }

    void OnBlueprintPreCompile(UBlueprint* Blueprint)
    {
        using namespace UnrealMvvm_Impl;

        // check that this Blueprint represents a View
        if (Blueprint != nullptr)
        {
            UClass* ViewModelClass = FViewModelClassSelectorHelper::GetViewModelClass(Blueprint);

            if (ViewModelClass != nullptr)
            {
                // make sure it has our UBaseViewBlueprintExtension
                // during compilation, the extension will do the rest of the work
                UBaseViewBlueprintExtension* Extension = UBaseViewBlueprintExtension::Request(Blueprint);

                // make sure the extension has valid ViewModel class
                if (Extension->GetViewModelClass() == nullptr)
                {
                    Extension->SetViewModelClass(ViewModelClass);
                }
            }
        }
    }

    void OnKismetPreCompile()
    {
        for (TWeakObjectPtr<UBlueprint> Blueprint : PendingBlueprints)
        {
            OnBlueprintPreCompile(Blueprint.Get());
        }

        PendingBlueprints.Reset();
    }

    TUniquePtr<FBlueprintCreateListener> BlueprintListener;
    TArray<TWeakObjectPtr<UBlueprint>> PendingBlueprints;
};


FBlueprintCreateListener::FBlueprintCreateListener(FUnrealMvvmEditorModule* InModule)
    : Module(InModule)
{}

FBlueprintCreateListener::~FBlueprintCreateListener()
{
    OnUObjectArrayShutdown();
}

void FBlueprintCreateListener::NotifyUObjectCreated(const class UObjectBase* Object, int32 Index)
{
    if (Object->GetClass()->IsChildOf<UBlueprint>())
    {
        Module->PendingBlueprints.Add((UBlueprint*)Object);
    }
}

void FBlueprintCreateListener::OnUObjectArrayShutdown()
{
    if (Module != nullptr)
    {
        GUObjectArray.RemoveUObjectCreateListener(this);
        Module = nullptr;
    }
}

IMPLEMENT_MODULE(FUnrealMvvmEditorModule, UnrealMvvmEditor)
