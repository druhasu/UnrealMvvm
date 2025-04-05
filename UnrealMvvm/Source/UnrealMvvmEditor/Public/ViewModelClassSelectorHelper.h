// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

class UBlueprint;
class UClass;
class FText;

class UNREALMVVMEDITOR_API FViewModelClassSelectorHelper
{
public:
    static UClass* GetViewModelClass(UBlueprint* Blueprint);

    static void SetViewModelClass(UBlueprint* Blueprint, UClass* ViewModelClass);

    static bool ValidateViewModelClass(UBlueprint* Blueprint, FText* OutError = nullptr);
};
