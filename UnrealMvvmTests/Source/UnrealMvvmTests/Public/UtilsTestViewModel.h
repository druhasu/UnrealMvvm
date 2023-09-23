// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/BaseViewModel.h"
#include "UtilsTestViewModel.generated.h"

UCLASS()
class UUtilsTestViewModel : public UBaseViewModel
{
	GENERATED_BODY()

public:
	void SetModel(const FString& InModel)
	{
		Model = InModel;
	}

	FString Model;
};