// Copyright Andrei Sudarikov. All Rights Reserved.

#include "BaseViewBlueprintExtension.h"
#include "Mvvm/Impl/ViewModelRegistry.h"
#include "Nodes/K2Node_InitViewModelDynamicBinding.h"
#include "KismetCompiler.h"

UBaseViewBlueprintExtension* UBaseViewBlueprintExtension::Request(UBlueprint* Blueprint)
{
    UBaseViewBlueprintExtension* Result = Get(Blueprint);

    if (!Result)
    {
        Result = NewObject<UBaseViewBlueprintExtension>(Blueprint);
        Blueprint->AddExtension(Result);
    }

    return Result;
}

UBaseViewBlueprintExtension* UBaseViewBlueprintExtension::Get(UBlueprint* Blueprint)
{
    const TObjectPtr<UBlueprintExtension>* FoundExtension = Blueprint->GetExtensions().FindByPredicate([](TObjectPtr<UBlueprintExtension> Ext)
    {
        return Ext->IsA<UBaseViewBlueprintExtension>();
    });

    return FoundExtension ? CastChecked<UBaseViewBlueprintExtension>(FoundExtension->Get()) : nullptr;
}

void UBaseViewBlueprintExtension::Remove(UBlueprint* Blueprint)
{
    UBaseViewBlueprintExtension* Extension = Get(Blueprint);
    if (Extension)
    {
        Blueprint->RemoveExtension(Extension);
    }
}

void UBaseViewBlueprintExtension::Serialize(FArchive& Ar)
{
    Super::Serialize(Ar);

    if (Ar.IsLoading())
    {
        // register ViewModel class association as soon as class is loaded
        TryRegisterViewModelClass();
    }
}

void UBaseViewBlueprintExtension::SetViewModelClass(UClass* InViewModelClass)
{
    ViewModelClass = InViewModelClass;

    // register new ViewModel class association
    TryRegisterViewModelClass();
}

void UBaseViewBlueprintExtension::HandleGenerateFunctionGraphs(FKismetCompilerContext* CompilerContext)
{
    UEdGraph* Graph = CompilerContext->SpawnIntermediateFunctionGraph(TEXT("RegisterViewModelClassStub"));

    // Function graph should always have entry node
    check(Graph->Nodes.Num() > 0)
    UK2Node* EntryNode = CastChecked<UK2Node>(Graph->Nodes[0]);

    // Create initializer node
    FGraphNodeCreator<UK2Node_InitViewModelDynamicBinding> Creator(*Graph);
    UK2Node_InitViewModelDynamicBinding* InitNode = Creator.CreateNode(false);
    Creator.Finalize();

    // Connect initializer node to function entry
    EntryNode->GetThenPin()->MakeLinkTo(InitNode->GetExecPin());
}

void UBaseViewBlueprintExtension::TryRegisterViewModelClass()
{
    // ViewClass may be nullptr if Blueprint did not compile successfully
    UClass* ViewClass = GetTypedOuter<UBlueprint>()->GeneratedClass;

    if (ViewClass && ViewModelClass)
    {
        UnrealMvvm_Impl::FViewModelRegistry::RegisterViewClass(ViewClass, ViewModelClass);
    }
}