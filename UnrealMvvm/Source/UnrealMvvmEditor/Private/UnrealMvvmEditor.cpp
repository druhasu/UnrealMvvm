// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Modules/ModuleManager.h"
#include "BlueprintGraphModule.h"
#include "UMGEditorModule.h"
#include "BlueprintViewModelNodeSpawner.h"
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
		FBlueprintGraphModule& BlueprintGraphModule = FModuleManager::LoadModuleChecked<FBlueprintGraphModule>("BlueprintGraph");
		IUMGEditorModule& UMGEditorModule = FModuleManager::LoadModuleChecked<IUMGEditorModule>("UMGEditor");

		FFilterDelegate Dlg = FFilterDelegate::CreateStatic(&UBlueprintViewModelNodeSpawner::FilterAction);
		FilterDelegateHandle = Dlg.GetHandle();
		BlueprintGraphModule.GetExtendedActionMenuFilters().Add(Dlg);

		UMGEditorModule.AddWidgetCustomizationExtender(ViewWidgetCustomizationExtender);
	}

	void ShutdownModule() override
	{
		FBlueprintGraphModule* BlueprintGraphModule = FModuleManager::GetModulePtr<FBlueprintGraphModule>("BlueprintGraph");
		IUMGEditorModule* UMGEditorModule = FModuleManager::GetModulePtr<IUMGEditorModule>("UMGEditor");

		if (BlueprintGraphModule)
		{
			BlueprintGraphModule->GetExtendedActionMenuFilters().RemoveAll([this](FFilterDelegate& Dlg)
			{
				return Dlg.GetHandle() == FilterDelegateHandle;
			});
		}

		if (UMGEditorModule)
		{
			UMGEditorModule->RemoveWidgetCustomizationExtender(ViewWidgetCustomizationExtender);
		}
	}

	TSharedRef<IBlueprintWidgetCustomizationExtender> ViewWidgetCustomizationExtender;
	FDelegateHandle FilterDelegateHandle;
};

IMPLEMENT_MODULE(FUnrealMvvmEditorModule, UnrealMvvmEditor)