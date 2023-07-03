// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "WorkflowOrientedApp/WorkflowTabFactory.h"

class FBlueprintEditor;

class FViewModelPropertiesSummoner : public FWorkflowTabFactory
{
public:
	FViewModelPropertiesSummoner(const TSharedPtr<FBlueprintEditor>& BlueprintEditor);

	TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const override;
	FText GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const override { return TabTooltip; }

	static const FName TabID;

private:
	TWeakPtr<FBlueprintEditor> WeakWidgetBlueprintEditor;
	FText TabTooltip;
};