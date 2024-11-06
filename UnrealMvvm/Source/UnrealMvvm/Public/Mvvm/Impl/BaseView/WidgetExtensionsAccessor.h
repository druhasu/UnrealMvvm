// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Containers/Array.h"
#include "UObject/ObjectPtr.h"
#include "Blueprint/UserWidget.h"

class UUserWidgetExtension;

namespace UnrealMvvm_Impl
{
    /*
     * Unfortunately we have to Hack and write directly to UUserWidget::Extensions, because it is not possible to perform some operations via public interface
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
}