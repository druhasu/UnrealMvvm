// Copyright Andrei Sudarikov. All Rights Reserved.

#include "BaseViewBlueprintExtension.h"
#include "WidgetBlueprintCompiler.h"
#include "Mvvm/Impl/BaseViewClassExtension.h"
#include "Mvvm/Impl/ViewModelRegistry.h"

void UBaseViewBlueprintExtension::HandleBeginCompilation(FWidgetBlueprintCompilerContext& InCreationContext)
{
    Context = &InCreationContext;
}

void UBaseViewBlueprintExtension::HandleFinishCompilingClass(UWidgetBlueprintGeneratedClass* Class)
{
    Super::HandleFinishCompilingClass(Class);

    if (ViewModelClass)
    {
        UnrealMvvm_Impl::FViewModelRegistry::RegisterViewClass(Context->Blueprint->GeneratedClass, ViewModelClass);

        if (Context->bIsFullCompile)
        {
            UBaseViewClassExtension* Extension = NewObject<UBaseViewClassExtension>(Class);
            Extension->ViewModelClass = ViewModelClass;
            Context->AddExtension(Class, Extension);
        }
    }
}