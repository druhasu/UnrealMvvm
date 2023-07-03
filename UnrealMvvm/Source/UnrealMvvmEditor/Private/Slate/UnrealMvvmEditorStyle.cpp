// Copyright Andrei Sudarikov. All Rights Reserved.

#include "UnrealMvvmEditorStyle.h"
#include "Misc/LazySingleton.h"
#include "Styling/SlateStyleMacros.h"

FUnrealMvvmEditorStyle& FUnrealMvvmEditorStyle::Get()
{
	return TLazySingleton<FUnrealMvvmEditorStyle>::Get();
}

void FUnrealMvvmEditorStyle::Shutdown()
{
	TLazySingleton<FUnrealMvvmEditorStyle>::TearDown();
}

FUnrealMvvmEditorStyle::FUnrealMvvmEditorStyle()
	: FSlateStyleSet("UnrealMvvmEditorStyle")
{
	const FVector2D Icon16x16(16.0f, 16.0f);

	SetContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate"));

	Set("ViewModelPropertiesTab.Icon", new IMAGE_BRUSH_SVG("Starship/Common/Spreadsheet", Icon16x16));

	FSlateStyleRegistry::RegisterSlateStyle(*this);
}

FUnrealMvvmEditorStyle::~FUnrealMvvmEditorStyle()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*this);
}