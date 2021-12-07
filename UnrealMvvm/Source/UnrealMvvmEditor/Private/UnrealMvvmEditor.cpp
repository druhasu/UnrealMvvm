// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "BlueprintGraphModule.h"
#include "BlueprintViewModelNodeSpawner.h"
#include "BaseViewDetailCustomization.h"

class FUnrealMvvmEditorModule : public IModuleInterface
{
public:
	using FFilterDelegate = FBlueprintGraphModule::FActionMenuRejectionTest;

	void StartupModule() override
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		FBlueprintGraphModule& BlueprintGraphModule = FModuleManager::LoadModuleChecked<FBlueprintGraphModule>("BlueprintGraph");

		PropertyModule.RegisterCustomClassLayout("BaseView", FOnGetDetailCustomizationInstance::CreateStatic(&FBaseViewDetailCustomization::MakeInstance));

		FFilterDelegate Dlg = FFilterDelegate::CreateStatic(&UBlueprintViewModelNodeSpawner::FilterAction);
		FilterDelegateHandle = Dlg.GetHandle();
		BlueprintGraphModule.GetExtendedActionMenuFilters().Add(Dlg);
	}

	void ShutdownModule() override
	{
		FPropertyEditorModule* PropertyModule = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor");
		FBlueprintGraphModule* BlueprintGraphModule = FModuleManager::GetModulePtr<FBlueprintGraphModule>("BlueprintGraph");

		if (PropertyModule)
		{
			PropertyModule->UnregisterCustomClassLayout("BaseView");
		}

		if (BlueprintGraphModule)
		{
			BlueprintGraphModule->GetExtendedActionMenuFilters().RemoveAll([this](FFilterDelegate& Dlg)
			{
				return Dlg.GetHandle() == FilterDelegateHandle;
			});
		}
	}

	FDelegateHandle FilterDelegateHandle;
};

IMPLEMENT_MODULE(FUnrealMvvmEditorModule, UnrealMvvmEditor)