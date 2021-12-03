// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "BaseViewDetailCustomization.h"

class FUnrealMvvmEditorModule : public IModuleInterface
{
public:
	void StartupModule() override
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

		PropertyModule.RegisterCustomClassLayout("BaseView", FOnGetDetailCustomizationInstance::CreateStatic(&FBaseViewDetailCustomization::MakeInstance));
	}

	void ShutdownModule() override
	{
		FPropertyEditorModule* PropertyModule = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor");

		if (PropertyModule)
		{
			PropertyModule->UnregisterCustomClassLayout("BaseView");
		}
	}
};

IMPLEMENT_MODULE(FUnrealMvvmEditorModule, UnrealMvvmEditor)