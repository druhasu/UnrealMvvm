// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Modules/ModuleManager.h"
#include "UMGEditorModule.h"
#include "ViewWidgetCustomizationExtender.h"

class FUnrealMvvmEditorModule : public IModuleInterface
{
public:
    using FFilterDelegate = FBlueprintGraphModule::FActionMenuRejectionTest;

    FUnrealMvvmEditorModule()
        : ViewWidgetCustomizationExtender(new FViewWidgetCustomizationExtender)
    {
        static_cast<FViewWidgetCustomizationExtender&>(*ViewWidgetCustomizationExtender).Init();
    }

    void StartupModule() override
    {
        IUMGEditorModule& UMGEditorModule = FModuleManager::LoadModuleChecked<IUMGEditorModule>("UMGEditor");

        UMGEditorModule.AddWidgetCustomizationExtender(ViewWidgetCustomizationExtender);
    }

    void ShutdownModule() override
    {
        IUMGEditorModule* UMGEditorModule = FModuleManager::GetModulePtr<IUMGEditorModule>("UMGEditor");

        if (UMGEditorModule)
        {
            UMGEditorModule->RemoveWidgetCustomizationExtender(ViewWidgetCustomizationExtender);
        }
    }

    TSharedRef<IBlueprintWidgetCustomizationExtender> ViewWidgetCustomizationExtender;
};

IMPLEMENT_MODULE(FUnrealMvvmEditorModule, UnrealMvvmEditor)