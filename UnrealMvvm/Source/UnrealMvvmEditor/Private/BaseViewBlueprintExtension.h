// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Blueprint/BlueprintExtension.h"
#include "BaseViewBlueprintExtension.generated.h"

/*
 * Blueprint extension class that stores ViewModel association and creates required nodes
 */
UCLASS()
class UBaseViewBlueprintExtension : public UBlueprintExtension
{
    GENERATED_BODY()

public:
    static UBaseViewBlueprintExtension* Request(UBlueprint* Blueprint);
    static UBaseViewBlueprintExtension* Get(UBlueprint* Blueprint);
    static void Remove(UBlueprint* Blueprint);

    void Serialize(FArchive& Ar) override;

    UClass* GetViewModelClass() const { return ViewModelClass; }
    void SetViewModelClass(UClass* InViewModelClass);

protected:
    void HandleGenerateFunctionGraphs(FKismetCompilerContext* CompilerContext) override;

private:
    void TryRegisterViewModelClass();

    UPROPERTY()
    UClass* ViewModelClass;
};
