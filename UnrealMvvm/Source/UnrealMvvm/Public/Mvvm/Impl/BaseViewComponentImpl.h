// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/BaseViewModel.h"
#include "Mvvm/Impl/BindEntry.h"
#include "Mvvm/Impl/ViewModelPropertyIterator.h"

class FViewModelPropertyBase;
class UBaseViewModel;

namespace UnrealMvvm_Impl
{

    struct FBlueprintPropertyChangeHandler : public UnrealMvvm_Impl::IPropertyChangeHandler
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

    struct FNoopPropertyChangeHandler : public UnrealMvvm_Impl::IPropertyChangeHandler
    {
        void Invoke(UBaseViewModel*, const FViewModelPropertyBase*) const override
        {
        }
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

        /* Fills BindEntries array */
        static void PrepareBindingsInternal(UClass* ViewModelClass, UObject* ViewObject, TArray<FBindEntry>& BindEntries)
        {
            // native bindings are handled in TBaseView

            // blueprint bindings
            for (FViewModelPropertyIterator Iter(ViewModelClass, false); Iter; ++Iter)
            {
                UFunction* Function = ViewObject->FindFunction(Iter->GetProperty()->GetCallbackName());

                if (Function)
                {
                    FBindEntry& Entry = BindEntries.Emplace_GetRef(Iter->GetProperty());
                    Entry.Handler.Emplace< FBlueprintPropertyChangeHandler >(ViewObject, Function);
                }
            }

            // add at least one bind to prevent this method from being called again
            if (BindEntries.Num() == 0)
            {
                FBindEntry& Entry = BindEntries.Emplace_GetRef(nullptr);
                Entry.Handler.Emplace< FNoopPropertyChangeHandler >();
            }
        }

        /* Called when ViewModel property changes */
        void OnPropertyChangedInternal(const FViewModelPropertyBase* Property, UBaseViewModel* ViewModel, TArray<FBindEntry>& BindEntries)
        {
            for (const FBindEntry& Bind : BindEntries)
            {
                if (Bind.Property == Property)
                {
                    Bind.Handler->Invoke(ViewModel, Property);
                }
            }
        }

        bool HasBindings(TArray<FBindEntry>& BindEntries) const
        {
            return BindEntries.Num() > 0 && BindEntries[0].Property != nullptr;
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
            UBaseViewModel* OldViewModel = ThisView()->ViewModel;

            if (OldViewModel && ThisView()->IsConstructed())
            {
                StopListening();
            }

            ThisView()->ViewModel = InViewModel;
            TryCallViewModelChanged(ThisView()->GetViewObject(), OldViewModel, InViewModel);

            if (InViewModel)
            {
                if (ThisView()->BindEntries.Num() == 0)
                {
                    PrepareBindings(ThisView()->ViewModel->GetClass());
                }

                if (ThisView()->IsConstructed())
                {
                    StartListening();
                }
            }
        }

        /* Fills BindEntries array */
        void PrepareBindings(UClass* ViewModelClass)
        {
            PrepareBindingsInternal(ViewModelClass, ThisView()->GetViewObject(), ThisView()->BindEntries);
        }

        /* Subscribes to ViewModel */
        void StartListening()
        {
            check(ThisView()->ViewModel); // this is ensured by caller

            if (HasBindings(ThisView()->BindEntries))
            {
                ThisView()->SubscriptionHandle = ThisView()->ViewModel->Subscribe(UBaseViewModel::FPropertyChangedDelegate::FDelegate::CreateUObject(ThisView(), &TView::OnPropertyChanged));

                for (auto& Bind : ThisView()->BindEntries)
                {
                    Bind.Handler->Invoke(ThisView()->ViewModel, Bind.Property);
                }
            }
        }

        /* Unsubscribes from ViewModel */
        void StopListening()
        {
            check(ThisView()->ViewModel); // this is ensured by caller

            if (HasBindings(ThisView()->BindEntries))
            {
                ThisView()->ViewModel->Unsubscribe(ThisView()->SubscriptionHandle);
            }
        }

        /* Called when ViewModel property changes */
        void OnPropertyChanged(const FViewModelPropertyBase* Property)
        {
            OnPropertyChangedInternal(Property, ThisView()->ViewModel, ThisView()->BindEntries);
        }

        const TView* ThisView() const { return static_cast<const TView*>(this); }
        TView* ThisView() { return static_cast<TView*>(this); }
    };

}