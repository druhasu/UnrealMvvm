// Copyright Andrei Sudarikov. All Rights Reserved.

#include "BaseViewBlueprintExtension.h"
#include "Mvvm/Impl/BaseView/ViewRegistry.h"
#include "Nodes/K2Node_InitViewModelDynamicBinding.h"
#include "Nodes/K2Node_ViewModelPropertyChanged.h"
#include "Nodes/ViewModelPropertyNodeHelper.h"
#include "KismetCompiler.h"
#include "Kismet2/BlueprintEditorUtils.h"

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
        Extension->SetViewModelClass(nullptr);
        Blueprint->RemoveExtension(Extension);
    }
}

UClass* UBaseViewBlueprintExtension::GetViewModelClass(UBlueprint* Blueprint)
{
    if (Blueprint)
    {
        UBaseViewBlueprintExtension* Extension = Get(Blueprint);

        return Extension != nullptr ? Extension->GetViewModelClass() : UnrealMvvm_Impl::FViewRegistry::GetViewModelClass(Blueprint->GeneratedClass);
    }

    return nullptr;
}

void UBaseViewBlueprintExtension::Serialize(FArchive& Ar)
{
    Super::Serialize(Ar);

    if (Ar.IsLoading())
    {
        // pickup legacy bindings by function name
        TryAddLegacyBindings();

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

TArray<FBlueprintBindingEntry> UBaseViewBlueprintExtension::CollectBlueprintBindings() const
{
    using namespace UnrealMvvm_Impl;

    TArray<FBlueprintBindingEntry> BlueprintBindings;

    TArray<UK2Node_ViewModelPropertyChanged*> Nodes;
    UBlueprint* Blueprint = GetTypedOuter<UBlueprint>();

    while (Blueprint != nullptr)
    {
        FBlueprintEditorUtils::GetAllNodesOfClass<UK2Node_ViewModelPropertyChanged>(Blueprint, Nodes);

        for (UK2Node_ViewModelPropertyChanged* Node : Nodes)
        {
            UEdGraphPin* ExecPin = Node->FindPin(UEdGraphSchema_K2::PN_Then);
            if (ExecPin != nullptr && ExecPin->HasAnyConnections() && FViewModelPropertyNodeHelper::IsPropertyPathValid(Node->PropertyPath, ViewModelClass))
            {
                FBlueprintBindingEntry& Entry = BlueprintBindings.AddDefaulted_GetRef();
                Entry.PropertyPath = Node->PropertyPath;
                Entry.FunctionName = Node->MakeCallbackName();
            }
        }

        Nodes.Reset();
        UClass* SuperClass = Blueprint->GeneratedClass != nullptr ? Blueprint->GeneratedClass->GetSuperClass() : nullptr;
        Blueprint = SuperClass != nullptr ? Cast<UBlueprint>(SuperClass->ClassGeneratedBy) : nullptr;
    }

    return BlueprintBindings;
}

void UBaseViewBlueprintExtension::HandleGenerateFunctionGraphs(FKismetCompilerContext* CompilerContext)
{
    UEdGraph* Graph = CompilerContext->SpawnIntermediateFunctionGraph(FString::Printf(TEXT("RegisterViewModelClassStub_%s"), *CompilerContext->TargetClass->GetName()));

    // Function graph should always have entry node
    check(Graph->Nodes.Num() > 0);
    UK2Node* EntryNode = CastChecked<UK2Node>(Graph->Nodes[0]);

    // Create initializer node
    FGraphNodeCreator<UK2Node_InitViewModelDynamicBinding> Creator(*Graph);
    UK2Node_InitViewModelDynamicBinding* InitNode = Creator.CreateNode(false);
    Creator.Finalize();

    // Connect initializer node to function entry
    EntryNode->GetThenPin()->MakeLinkTo(InitNode->GetExecPin());

    // make sure that ViewModel class association is registered
    // some nodes need to know ViewModel class of the Blueprint during compilation
    // we need to do this here for correct handling of Blueprint duplication
    // we cannot handle it inside our PostDuplicate because compilation happens first
    TryRegisterViewModelClass();
}

void UBaseViewBlueprintExtension::TryRegisterViewModelClass()
{
    // ViewClass may be nullptr if Blueprint did not compile successfully
    UClass* ViewClass = GetTypedOuter<UBlueprint>()->GeneratedClass;

    if (ViewClass)
    {
        if (ViewModelClass)
        {
            UnrealMvvm_Impl::FViewRegistry::RegisterViewClass(ViewClass, ViewModelClass);
        }
        else
        {
            UnrealMvvm_Impl::FViewRegistry::UnregisterViewClass(ViewClass);
        }
    }
}

void UBaseViewBlueprintExtension::TryAddLegacyBindings()
{
    /*
     * This functions collects bindings from existing nodes and pushes them into existing UViewModelDynamicBinding
     * In earlier versions we just looked for functions with "specific" names in runtime to find Bindings
     * Now they are explicitly stored inside a UViewModelDynamicBinding, so we need to put them there
     * Actual conversion is performed by the UK2Node_ViewModelPropertyChanged
     */
    using namespace UnrealMvvm_Impl;

    UClass* ViewClass = GetTypedOuter<UBlueprint>()->GeneratedClass;

    if (ViewClass != nullptr && ViewModelClass != nullptr)
    {
        TArray<FBlueprintBindingEntry> BlueprintBindings = CollectBlueprintBindings();

        // push newly created bindings to UViewModelDynamicBinding to make Blueprint work without recompilation and resaving
        UViewModelDynamicBinding* ViewModelDynamicBinding = static_cast<UViewModelDynamicBinding*>(UBlueprintGeneratedClass::GetDynamicBindingObject(ViewClass, UViewModelDynamicBinding::StaticClass()));
        if (ViewModelDynamicBinding == nullptr)
        {
            // in some even older versions we did not use UViewModelDynamicBinding to save View to ViewModel associations, so the class may not have it
            // if it is the case, create new UViewModelDynamicBinding right now. It will be regenerated during next compile and save
            if (UBlueprintGeneratedClass* BPGC = Cast<UBlueprintGeneratedClass>(ViewClass))
            {
                ViewModelDynamicBinding = NewObject<UViewModelDynamicBinding>(BPGC);
                BPGC->DynamicBindingObjects.Add(ViewModelDynamicBinding);
            }
        }

        if (ViewModelDynamicBinding != nullptr)
        {
            ViewModelDynamicBinding->BlueprintBindings = BlueprintBindings;
        }
    }
}
