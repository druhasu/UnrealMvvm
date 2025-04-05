// Copyright Andrei Sudarikov. All Rights Reserved.

#include "ViewModelClassSelectorHelper.h"
#include "Engine/Blueprint.h"
#include "Mvvm/Impl/BaseView/ViewRegistry.h"
#include "BaseViewBlueprintExtension.h"

UClass* FViewModelClassSelectorHelper::GetViewModelClass(UBlueprint* Blueprint)
{
    if (Blueprint)
    {
        UBaseViewBlueprintExtension* Extension = UBaseViewBlueprintExtension::Get(Blueprint);

        return Extension != nullptr ? Extension->GetViewModelClass() : UnrealMvvm_Impl::FViewRegistry::GetViewModelClass(Blueprint->GeneratedClass);
    }

    return nullptr;
}

void FViewModelClassSelectorHelper::SetViewModelClass(UBlueprint* Blueprint, UClass* ViewModelClass)
{
    if (ViewModelClass != nullptr)
    {
        UBaseViewBlueprintExtension* Extension = UBaseViewBlueprintExtension::Request(Blueprint);
        Extension->SetViewModelClass(ViewModelClass);
    }
    else
    {
        UBaseViewBlueprintExtension::Remove(Blueprint);
    }
}

bool FViewModelClassSelectorHelper::ValidateViewModelClass(UBlueprint* Blueprint, FText* OutError)
{
    UBaseViewBlueprintExtension* Extension = UBaseViewBlueprintExtension::Get(Blueprint);
    if (Extension == nullptr)
    {
        // it is perfectly fine if blueprint does not have BaseView extension
        return true;
    }

    UClass* ViewModelClass = Extension->GetViewModelClass();
    if (ViewModelClass == nullptr)
    {
        // it is not expected, but acceptable
        return true;
    }

    UClass* ParentClass = Blueprint->ParentClass;
    UClass* ParentViewModelClass = UnrealMvvm_Impl::FViewRegistry::GetViewModelClass(ParentClass);

    if (ParentViewModelClass != nullptr && !ViewModelClass->IsChildOf(ParentViewModelClass))
    {
        if (OutError != nullptr)
        {
            // this Blueprint has ViewModel class set, but it is not a subclass of its Parent class ViewModel
            *OutError = FText::FormatOrdered(
                NSLOCTEXT("UnrealMvvm", "Error.ViewModelClassInvalid.NotSubclassOfParent", "ViewModel class must be subclass of '{1}'. '{0}' is not"),
                FText::FromString(ViewModelClass->GetName()),
                FText::FromString(ParentViewModelClass->GetName())
            );
        }

        return false;
    }

    return true;
}
