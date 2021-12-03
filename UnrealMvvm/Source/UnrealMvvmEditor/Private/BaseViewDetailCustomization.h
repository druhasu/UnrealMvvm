// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "IDetailCustomization.h"

class UNREALMVVMEDITOR_API FBaseViewDetailCustomization : public IDetailCustomization
{
public:
    static TSharedRef<IDetailCustomization> MakeInstance();

    void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
    FReply HandleAddOrViewEventForProperty(UClass* ViewModelClass, FName PropertyName);
    int32 HandleAddOrViewIndexForButton(UClass* ViewModelClass, FName PropertyName) const;
    class UK2Node_ViewModelPropertyChanged* FindEventNode(UClass* ViewModelClass, FName PropertyName) const;

    UBlueprint* Blueprint;
};