// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"

class UK2Node_ViewModelPropertyChanged;
class SWrapBox;

namespace UnrealMvvm_Impl
{
    struct FViewModelPropertyReflection;
}

class SViewModelBindingEntry : public SCompoundWidget
{
public:
    using ThisClass = SViewModelBindingEntry;

    SLATE_BEGIN_ARGS(SViewModelBindingEntry)
        {}

        SLATE_EVENT(FSimpleDelegate, OnRemoveClicked)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView, UK2Node_ViewModelPropertyChanged* InNode);

private:
    TSharedRef<SWidget> MakeContentWidget();
    TSharedRef<SWidget> MakePropertyPathWidget();
    void AddPropertyPathEntry(SWrapBox& WrapBox, FName PropertyName, const UnrealMvvm_Impl::FViewModelPropertyReflection* Reflection);

    FReply HandleViewButtonClicked();

    TWeakObjectPtr<UK2Node_ViewModelPropertyChanged> Node;
    FSimpleDelegate RemoveDelegate;
};
