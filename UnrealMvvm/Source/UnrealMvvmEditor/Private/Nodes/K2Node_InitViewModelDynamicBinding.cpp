// Copyright Andrei Sudarikov. All Rights Reserved.

#include "K2Node_InitViewModelDynamicBinding.h"
#include "BaseViewBlueprintExtension.h"
#include "Mvvm/Impl/Binding/ViewModelDynamicBinding.h"
#include "Mvvm/Impl/BaseView/ViewRegistry.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "KismetCompilerMisc.h"
#include "EdGraphSchema_K2.h"

FNodeHandlingFunctor* UK2Node_InitViewModelDynamicBinding::CreateNodeHandler(class FKismetCompilerContext& CompilerContext) const
{
    // this node does nothing in runtime (and should be never called)
    // return passthru handler so compiler won't flag it as unsupported
    return new FKCHandler_Passthru(CompilerContext);
}

UClass* UK2Node_InitViewModelDynamicBinding::GetDynamicBindingClass() const
{
    return UViewModelDynamicBinding::StaticClass();
}

void UK2Node_InitViewModelDynamicBinding::RegisterDynamicBinding(UDynamicBlueprintBinding* BindingObject) const
{
    UViewModelDynamicBinding* Binding = CastChecked<UViewModelDynamicBinding>(BindingObject);
    UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForNodeChecked(this);
    UBaseViewBlueprintExtension* Extension = UBaseViewBlueprintExtension::Get(Blueprint);

    // Should never be null, because this node may only be created by that Extension
    check(Extension);

    // Copy ViewModelClass from blueprint into DynamicBinding object
    // We need to do this because UBlueprint is not cooked and we need to know about this class in runtime
    Binding->ViewModelClass = Extension->GetViewModelClass();

    // ViewModelClass may be nullptr if ViewModel was removed from the project and not redirected
    if (Binding->ViewModelClass != nullptr)
    {
        // Collect all ViewModel property bindings into DynamicBinding object
        Binding->BlueprintBindings = Extension->CollectBlueprintBindings();

        // register View class to generate proper ViewModel property bindings
        UnrealMvvm_Impl::FViewRegistry::RegisterViewClass(Blueprint->GeneratedClass, Binding->ViewModelClass);
    }
}

void UK2Node_InitViewModelDynamicBinding::AllocateDefaultPins()
{
    // create basic execution pins so this node can be connected to others
    CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);
    CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);
}
