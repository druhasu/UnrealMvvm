// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Modules/ModuleManager.h"
#include "Mvvm/BaseView.h"
#include "Mvvm/Impl/WidgetExtensionsAccessor.h"
#include "Slate/ViewModelPropertiesSummoner.h"
#include "Slate/UnrealMvvmEditorStyle.h"
#include "BaseViewBlueprintExtension.h"
#include "UMGEditorModule.h"
#include "BlueprintEditorModule.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "BlueprintModes/WidgetBlueprintApplicationMode.h"
#include "BlueprintModes/WidgetBlueprintApplicationModes.h"
#include "WidgetBlueprint.h"
#include "Blueprint/WidgetTree.h"

class FUnrealMvvmEditorModule : public IModuleInterface
{
public:
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
    }

    void ShutdownModule() override
    {
        FUnrealMvvmEditorStyle::Shutdown();

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
        if (ApplicationMode.GetModeName() == FWidgetBlueprintApplicationModes::DesignerMode)
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
        using namespace UnrealMvvm_Impl;

        UWidgetBlueprint* SavedBlueprint = Cast<UWidgetBlueprint>(Asset);
        if (!SavedBlueprint)
        {
            // it's not a widget, ignore
            return;
        }

        UClass* NativeViewClass = FBlueprintEditorUtils::GetNativeParent(SavedBlueprint);
        UClass* ViewModelClass = FViewModelRegistry::GetViewModelClass(NativeViewClass);

        if (NativeViewClass == nullptr)
        {
            // this widget was saved with base class that no longer exist, ignore it
            return;
        }

        if (NativeViewClass->IsChildOf<UBaseView>() || ViewModelClass == nullptr)
        {
            // it's either BaseView (that handles everything itself) or not a View at all, ignore
            return;
        }

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
};

IMPLEMENT_MODULE(FUnrealMvvmEditorModule, UnrealMvvmEditor)