// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Mvvm/Impl/BaseViewExtension.h"
#include "Mvvm/BaseViewModel.h"
#include "Mvvm/Impl/ViewModelPropertyIterator.h"
#include "Blueprint/UserWidget.h"

const FName UBaseViewExtension::ViewModelChangedFunctionName{ "OnVM_ViewModelChanged" };

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

/*
 * Unfortunately we have to Hack and write directly to UUserWidget::Extensions, because it is not possible to use AddExtension<> method from constructor.
 * These classes allow us to write into private variable.
 */
template<typename Accessor, typename Accessor::Member Member>
struct AccessPrivate
{
    friend typename Accessor::Member GetPrivate(Accessor InAccessor) { return Member; }
};

struct ExtensionsAccessor
{
    using Member = TArray<TObjectPtr<UUserWidgetExtension>> UUserWidget::*;
    friend Member GetPrivate(ExtensionsAccessor);
};

template struct AccessPrivate<ExtensionsAccessor, &UUserWidget::Extensions>;

void UBaseViewExtension::Construct()
{
    if (ViewModel)
    {
        // View is constructed (i.e. visible), start listening and update current state
        StartListening();
    }
}

void UBaseViewExtension::Destruct()
{
    if (ViewModel)
    {
        // View is no longer attached to anything, stop listening to ViewModel
        StopListening();
    }
}

UBaseViewExtension* UBaseViewExtension::Request(UUserWidget* Widget)
{
    if (Widget->HasAnyFlags(RF_ClassDefaultObject))
    {
        return nullptr;
    }

    UBaseViewExtension* Result = Widget->GetExtension<UBaseViewExtension>();

    if (!Result)
    {
        if(FUObjectThreadContext::Get().IsInConstructor)
        {
            // call NewObject with a name to allow object creation inside a constructor
            Result = NewObject<UBaseViewExtension>(Widget, "BaseViewExtension");
            (Widget->*GetPrivate(ExtensionsAccessor())).Add(Result);
        }
        else
        {
            Result = Widget->AddExtension<UBaseViewExtension>();
        }
    }

    return Result;
}

UBaseViewExtension* UBaseViewExtension::Get(const UUserWidget* Widget)
{
    return Widget->GetExtension<UBaseViewExtension>();
}

void UBaseViewExtension::SetViewModelInternal(UBaseViewModel* InViewModel)
{
    UUserWidget* ThisView = GetUserWidget();

    UBaseViewModel* OldViewModel = ViewModel;

    if (OldViewModel && ThisView->IsConstructed())
    {
        StopListening();
    }

    ViewModel = InViewModel;
    TryCallViewModelChanged(OldViewModel, InViewModel);

    if (InViewModel)
    {
        if (BindEntries.Num() == 0)
        {
            PrepareBindings(ViewModel->GetClass());
        }

        if (ThisView->IsConstructed())
        {
            StartListening();
        }
    }
}

void UBaseViewExtension::PrepareBindings(UClass* ViewModelClass)
{
    // native bindings are handled in TBaseView

    // blueprint bindings
    UUserWidget* ThisView = GetUserWidget();
    for (UnrealMvvm_Impl::FViewModelPropertyIterator Iter(ViewModelClass, false); Iter; ++Iter)
    {
        UFunction* Function = ThisView->FindFunction(Iter->GetProperty()->GetCallbackName());

        if (Function)
        {
            UnrealMvvm_Impl::FBindEntry& Entry = BindEntries.Emplace_GetRef(Iter->GetProperty());
            Entry.Handler.Emplace< FBlueprintPropertyChangeHandler >(ThisView, Function);
        }
    }

    // add at least one bind to prevent this method from being called again
    if (BindEntries.Num() == 0)
    {
        UnrealMvvm_Impl::FBindEntry& Entry = BindEntries.Emplace_GetRef(nullptr);
        Entry.Handler.Emplace< FNoopPropertyChangeHandler >();
    }
}

void UBaseViewExtension::StartListening()
{
    check(ViewModel); // this is ensured by caller

    if (HasBindings())
    {
        SubscriptionHandle = ViewModel->Subscribe(UBaseViewModel::FPropertyChangedDelegate::FDelegate::CreateUObject(this, &ThisClass::OnPropertyChanged));

        for (auto& Bind : BindEntries)
        {
            Bind.Handler->Invoke(ViewModel, Bind.Property);
        }
    }
}

void UBaseViewExtension::StopListening()
{
    check(ViewModel); // this is ensured by caller

    if (HasBindings())
    {
        ViewModel->Unsubscribe(SubscriptionHandle);
    }
}

void UBaseViewExtension::TryCallViewModelChanged(UBaseViewModel* OldViewModel, UBaseViewModel* NewViewModel)
{
    UUserWidget* Widget = GetUserWidget();
    UClass* Class = Widget->GetClass();
    UFunction* Function = Class->FindFunctionByName(ViewModelChangedFunctionName);

    if (Function)
    {
        auto Parms = MakeTuple(OldViewModel, NewViewModel);
        Widget->ProcessEvent(Function, &Parms);
    }
}

void UBaseViewExtension::OnPropertyChanged(const FViewModelPropertyBase* Property)
{
    for (const UnrealMvvm_Impl::FBindEntry& Bind : BindEntries)
    {
        if (Bind.Property == Property)
        {
            Bind.Handler->Invoke(ViewModel, Property);
        }
    }
}