// Copyright Andrei Sudarikov. All Rights Reserved.

#include "K2Node_ViewModelChanged.h"
#include "Mvvm/BaseViewModel.h"
#include "Mvvm/Impl/BaseView/BaseViewComponentImpl.h"
#include "Mvvm/Impl/BaseView/ViewRegistry.h"
#include "ViewModelClassSelectorHelper.h"
#include "GraphEditorSettings.h"
#include "EdGraphSchema_K2.h"
#include "KismetCompiler.h"
#include "K2Node_CustomEvent.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintEventNodeSpawner.h"
#include "Misc/EngineVersionComparison.h"

const FName UK2Node_ViewModelChanged::OldViewModelPinName{ "OldViewModel" };
const FName UK2Node_ViewModelChanged::NewViewModelPinName{ "NewViewModel" };
const FText UK2Node_ViewModelChanged::NodeCategory = FText::FromString("View Model");

void UK2Node_ViewModelChanged::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
    using namespace UnrealMvvm_Impl;

    UEdGraphPin* ExecPin = FindPin(UEdGraphSchema_K2::PN_Then);
    UEdGraphPin* OldViewModelPin = FindPin(OldViewModelPinName);
    UEdGraphPin* NewViewModelPin = FindPin(NewViewModelPinName);

    if (!ExecPin->HasAnyConnections())
    {
        return;
    }

    UClass* ViewModelClass = FViewModelClassSelectorHelper::GetViewModelClass(GetBlueprint());
    auto MakePinInfo = [&](const FName& NewName)
    {
        TSharedPtr<FUserPinInfo> Result = MakeShared<FUserPinInfo>();
        Result->DesiredPinDirection = EGPD_Output;
        Result->PinName = NewName;
        Result->PinType.PinCategory = UEdGraphSchema_K2::PC_Object;
        Result->PinType.PinSubCategoryObject = ViewModelClass;

        return Result;
    };

#if UE_VERSION_OLDER_THAN(5,4,0)
    UK2Node_CustomEvent* CustomEvent = CompilerContext.SpawnIntermediateEventNode<UK2Node_CustomEvent>(this, ExecPin, SourceGraph);
#else
    UK2Node_CustomEvent* CustomEvent = CompilerContext.SpawnIntermediateNode<UK2Node_CustomEvent>(this, SourceGraph);
#endif
    CustomEvent->UserDefinedPins.Emplace(MakePinInfo(OldViewModelPinName));
    CustomEvent->UserDefinedPins.Emplace(MakePinInfo(NewViewModelPinName));
    CustomEvent->CustomFunctionName = FBaseViewComponentImpl::ViewModelChangedFunctionName;
    CustomEvent->AllocateDefaultPins();

    const UEdGraphSchema_K2* Schema = CompilerContext.GetSchema();
    CompilerContext.MovePinLinksToIntermediate(*ExecPin, *Schema->FindExecutionPin(*CustomEvent, EGPD_Output));
    CompilerContext.MovePinLinksToIntermediate(*OldViewModelPin, *CustomEvent->FindPin(OldViewModelPinName));
    CompilerContext.MovePinLinksToIntermediate(*NewViewModelPin, *CustomEvent->FindPin(NewViewModelPinName));
}

void UK2Node_ViewModelChanged::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
    UClass* ActionKey = GetClass();

    if (ActionRegistrar.IsOpenForRegistration(ActionKey))
    {
        UBlueprintNodeSpawner* Spawner = UBlueprintNodeSpawner::Create<ThisClass>();
        Spawner->DefaultMenuSignature.Category = NodeCategory;
        ActionRegistrar.AddBlueprintAction(ActionKey, Spawner);
    }
}

bool UK2Node_ViewModelChanged::IsActionFilteredOut(class FBlueprintActionFilter const& Filter)
{
    using namespace UnrealMvvm_Impl;

    for (UBlueprint* Blueprint : Filter.Context.Blueprints)
    {
        UClass* ViewModelClass = FViewModelClassSelectorHelper::GetViewModelClass(Blueprint);

        if (!ViewModelClass)
        {
            return true;
        }
    }

    return false;
}

void UK2Node_ViewModelChanged::PostReconstructNode()
{
    Super::PostReconstructNode();
    UpdatePinTypes();
}

void UK2Node_ViewModelChanged::AllocateDefaultPins()
{
    using namespace UnrealMvvm_Impl;

    CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);
    CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Object, UBaseViewModel::StaticClass(), OldViewModelPinName);
    CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Object, UBaseViewModel::StaticClass(), NewViewModelPinName);
    UpdatePinTypes();

    // there is no convenient Init method in K2Node, so we have to resubscribe every time pins are created
    FViewRegistry::ViewModelClassChanged.RemoveAll(this);
    FViewRegistry::ViewModelClassChanged.AddUObject(this, &ThisClass::OnViewClassChanged);
}

FText UK2Node_ViewModelChanged::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
    static const FText NodeTitle = FText::FromString("On ViewModel Changed");
    return NodeTitle;
}

FLinearColor UK2Node_ViewModelChanged::GetNodeTitleColor() const
{
    return GetDefault<UGraphEditorSettings>()->EventNodeTitleColor;
}

FSlateIcon UK2Node_ViewModelChanged::GetIconAndTint(FLinearColor& OutColor) const
{
    static FSlateIcon Icon("EditorStyle", "GraphEditor.Event_16x");
    return Icon;
}

UClass* UK2Node_ViewModelChanged::GetViewClass() const
{
    return GetBlueprint()->GeneratedClass;
}

void UK2Node_ViewModelChanged::UpdatePinTypes()
{
    using namespace UnrealMvvm_Impl;

    UClass* ViewModelClass = FViewModelClassSelectorHelper::GetViewModelClass(GetBlueprint());
    FindPin(OldViewModelPinName)->PinType.PinSubCategoryObject = ViewModelClass;
    FindPin(NewViewModelPinName)->PinType.PinSubCategoryObject = ViewModelClass;
}

void UK2Node_ViewModelChanged::OnViewClassChanged(UClass* ViewClass, UClass* ViewModelClass)
{
    if (GetViewClass() == ViewClass)
    {
        UpdatePinTypes();
    }
}
