// Copyright Andrei Sudarikov. All Rights Reserved.

#include "ViewModelPropertiesSummoner.h"
#include "UnrealMvvmEditorStyle.h"
#include "BlueprintEditor.h"
#include "SViewModelPropertiesPanel.h"

const FName FViewModelPropertiesSummoner::TabID(TEXT("ViewModelPropertiesTab"));

FViewModelPropertiesSummoner::FViewModelPropertiesSummoner(const TSharedPtr<FBlueprintEditor>& BlueprintEditor)
	: FWorkflowTabFactory(TabID, BlueprintEditor)
	, WeakWidgetBlueprintEditor(BlueprintEditor)
{
	TabLabel = NSLOCTEXT("UnrealMvvm", "Tab.ViewModelProperties.Name", "View Properties");
	TabTooltip = NSLOCTEXT("UnrealMvvm", "Tab.ViewModelProperties.Tooltip", "Allows selecting ViewModel for your View and binding to properties");
	TabIcon = FSlateIcon(FUnrealMvvmEditorStyle::Get().GetStyleSetName(), "ViewModelPropertiesTab.Icon");

	bIsSingleton = true;

	ViewMenuDescription = NSLOCTEXT("UnrealMvvm", "Tab.ViewModelProperties.MenuDesc", "Allows selecting ViewModel for your View and binding to properties");
	ViewMenuTooltip = NSLOCTEXT("UnrealMvvm", "Tab.ViewModelProperties.MenuToolTip", "Show the ViewModel Propertes tab");
}

TSharedRef<SWidget> FViewModelPropertiesSummoner::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	return SNew(SViewModelPropertiesPanel, WeakWidgetBlueprintEditor.Pin());
}