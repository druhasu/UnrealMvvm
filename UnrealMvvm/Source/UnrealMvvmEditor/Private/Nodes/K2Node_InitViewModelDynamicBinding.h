// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "K2Node.h"
#include "K2Node_InitViewModelDynamicBinding.generated.h"

/*
 * Helper node that creates and initializes ViewModelDynamicBinding for View class
 */
UCLASS()
class UK2Node_InitViewModelDynamicBinding : public UK2Node
{
	GENERATED_BODY()

public:
	//~ Begin UK2Node Interface
	FNodeHandlingFunctor* CreateNodeHandler(class FKismetCompilerContext& CompilerContext) const override;
	UClass* GetDynamicBindingClass() const override;
	void RegisterDynamicBinding(UDynamicBlueprintBinding* BindingObject) const override;
	//~ End UK2Node Interface

	//~ Begin UEdGraphNode Interface
	void AllocateDefaultPins() override;
	//~ End UEdGraphNode Interface
};