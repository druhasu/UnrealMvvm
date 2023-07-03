// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"

/**
 *
 */
class FUnrealMvvmEditorStyle : public FSlateStyleSet
{

public:
	static FUnrealMvvmEditorStyle& Get();
	static void Shutdown();

private:
	friend class FLazySingleton;

	FUnrealMvvmEditorStyle();
	~FUnrealMvvmEditorStyle();
};
