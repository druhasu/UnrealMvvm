// Copyright Andrei Sudarikov. All Rights Reserved.

#include "BaseViewBlueprintExtension.h"
#include "Mvvm/Impl/BaseView/ViewRegistry.h"
#include "Mvvm/Impl/Property/ViewModelRegistry.h"
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
        Extension->SetViewModelClass(nullptr);
        Blueprint->RemoveExtension(Extension);
    }
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

void UBaseViewBlueprintExtension::HandleGenerateFunctionGraphs(FKismetCompilerContext* CompilerContext)
{
    UEdGraph* Graph = CompilerContext->SpawnIntermediateFunctionGraph(TEXT("RegisterViewModelClassStub"));

    // Function graph should always have entry node
    check(Graph->Nodes.Num() > 0);
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
     * This function upgrades bindings in "legacy" format to new format
     * Previously, each property had specified callback function name, which used for handling its change events
     * In new version, function name is generated for each "Binding" entry and stored inside the blueprint
     * So here we search for functions with "legacy" names and add appropriate new bindings for them.
     * When blueprint is recompiled and saved, those functions will be removed and replaced with new ones
     */
    using namespace UnrealMvvm_Impl;

    UClass* ViewClass = GetTypedOuter<UBlueprint>()->GeneratedClass;

    if (ViewClass != nullptr && ViewModelClass != nullptr)
    {
        const TArray<FViewModelPropertyReflection>* Properties = FViewModelRegistry::GetAllProperties().Find(ViewModelClass);

        if (Properties != nullptr)
        {
            for (const FViewModelPropertyReflection& Reflection : *Properties)
            {
                FName FunctionName = Reflection.GetProperty()->GetLegacyCallbackName();

                UFunction* Function = ViewClass->FindFunctionByName(FunctionName);
                if (Function != nullptr)
                {
                    BlueprintBindings.Add(FBlueprintBindingEntry{ { Reflection.GetProperty()->GetName() }, FunctionName });
                }
            }
        }

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
