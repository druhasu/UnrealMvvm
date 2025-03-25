// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/Impl/BaseView/ViewRegistry.h"
#include "Mvvm/Impl/BaseView/ViewChangeTracker.h"
#include "Mvvm/Impl/Binding/IPropertyChangeHandler.h"
#include "Mvvm/Impl/Binding/ViewModelDynamicBinding.h"
#include "Mvvm/Impl/Binding/BindingWorker.h"
#include "Engine/BlueprintGeneratedClass.h"

class FViewModelPropertyBase;
class UBaseViewModel;

namespace UnrealMvvm_Impl
{

    struct FBlueprintPropertyChangeHandler : public IPropertyChangeHandler
    {
        FBlueprintPropertyChangeHandler(UObject* InBaseView, UFunction* InFunction)
            : BaseView(InBaseView), Function(InFunction)
        {
        }

        void Invoke(UBaseViewModel*, const FViewModelPropertyBase*) const override
        {
            BaseView->ProcessEvent(Function, nullptr);
        }

        UObject* BaseView;
        UFunction* Function;
    };

    template<typename TOwner, typename TViewModel, typename TComponent>
    class TBaseViewImplWithComponent;

    class UNREALMVVM_API FBaseViewComponentImpl
    {
    public:
        /* Calls ViewModelChanged event, if it exist in blueprint class */
        static void TryCallViewModelChanged(UObject* ViewObject, UBaseViewModel* OldViewModel, UBaseViewModel* NewViewModel)
        {
            UClass* Class = ViewObject->GetClass();
            UFunction* Function = Class->FindFunctionByName(ViewModelChangedFunctionName);

            if (Function)
            {
                auto Parms = MakeTuple(OldViewModel, NewViewModel);
                ViewObject->ProcessEvent(Function, &Parms);
            }
        }

        /* Adds bindings to BindingWorker */
        static void PrepareBindindsInternal(UObject* ViewObject, FBindingWorker& Worker)
        {
            const FBindingConfiguration* FoundConfiguration = FViewRegistry::GetBindingConfiguration(ViewObject->GetClass());
            if (!FoundConfiguration)
            {
                return;
            }

            Worker.Init(ViewObject, *FoundConfiguration);

            // native bindings
            FViewRegistry::FBindingsCollectorPtr BindingsCollector = FViewRegistry::GetBindingsCollector(ViewObject->GetClass());
            if (BindingsCollector != nullptr)
            {
                BindingsCollector(*ViewObject);
            }

            // blueprint bindings
            UViewModelDynamicBinding* ViewModelDynamicBinding = static_cast<UViewModelDynamicBinding*>(UBlueprintGeneratedClass::GetDynamicBindingObject(ViewObject->GetClass(), UViewModelDynamicBinding::StaticClass()));
            if (ViewModelDynamicBinding != nullptr)
            {
                for (const FBlueprintBindingEntry& Binding : ViewModelDynamicBinding->BlueprintBindings)
                {
                    UFunction* Function = ViewObject->FindFunction(Binding.FunctionName);
                    Worker.AddBindingHandler<FBlueprintPropertyChangeHandler>(Binding.PropertyPath, ViewObject, Function);
                }
            }
        }

        static void InvokeStartListening(UObject* ViewObject, FBindingWorker& Worker)
        {
            UnrealMvvm_Impl::FViewInitializationScope Scope(ViewObject, Worker.GetViewModel());
            Worker.StartListening();
        }

        /* Name of UFunction to call when ViewModel changes */
        static FName ViewModelChangedFunctionName;
    };

    /* Contains common functions between Widget views and Actor views */
    template <typename TView>
    class TBaseViewComponentImpl : protected FBaseViewComponentImpl
    {
    public:

        /* Sets ViewModel and call required events */
        void SetViewModelInternal(UBaseViewModel* InViewModel)
        {
            TView* ThisView = static_cast<TView*>(this);

            UBaseViewModel* OldViewModel = ThisView->ViewModel;

            if (OldViewModel && ThisView->IsConstructed())
            {
                ThisView->BindingWorker.StopListening();
            }

            ThisView->ViewModel = InViewModel;
            ThisView->BindingWorker.SetViewModel(InViewModel);
            TryCallViewModelChanged(ThisView->GetViewObject(), OldViewModel, InViewModel);

            if (InViewModel)
            {
                if (ThisView->IsConstructed())
                {
                    InvokeStartListening(ThisView->GetViewObject(), ThisView->BindingWorker);
                }
            }
        }
    };

}
