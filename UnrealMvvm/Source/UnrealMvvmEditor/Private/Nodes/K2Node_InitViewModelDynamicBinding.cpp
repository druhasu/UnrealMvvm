// Copyright Andrei Sudarikov. All Rights Reserved.

#include "K2Node_InitViewModelDynamicBinding.h"
#include "BaseViewBlueprintExtension.h"
#include "Mvvm/Impl/ViewModelDynamicBinding.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "KismetCompilerMisc.h"

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
    // Copy ViewModelClass from blueprint into DynamicBinding object
    // We need to do this because UBlueprint is not cooked and we need to know about this class in runtime
    auto* Binding = CastChecked<UViewModelDynamicBinding>(BindingObject);
    auto* Extension = UBaseViewBlueprintExtension::Get(FBlueprintEditorUtils::FindBlueprintForNodeChecked(this));

    // Should never be null, because this node may only be created by that Extension
    check(Extension);

    Binding->ViewModelClass = Extension->GetViewModelClass();
}

void UK2Node_InitViewModelDynamicBinding::AllocateDefaultPins()
{
    // create basic execution pins so this node can be connected to others
    CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);
    CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);
}