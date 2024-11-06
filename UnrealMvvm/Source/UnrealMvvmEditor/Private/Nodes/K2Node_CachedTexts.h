// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "K2Node.h"
#include "EdGraph/EdGraphNodeUtils.h"

#include "K2Node_CachedTexts.generated.h"

/*
 * Base class for Nodes that want to cache their Title and Tooltip
 */
UCLASS(Abstract)
class UNREALMVVMEDITOR_API UK2Node_CachedTexts : public UK2Node
{
    GENERATED_BODY()

public:
    //~ Begin UEdGraphNode Interface
    FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
    FText GetTooltipText() const override;
    //~ End UEdGraphNode Interface

    //~ Begin UObject Interface
    bool Modify(bool bAlwaysMarkDirty = true) override;
    //~ End UObject Interface

protected:
    virtual FText GetNodeTitleForCache(ENodeTitleType::Type TitleType) const { return FText::GetEmpty(); };
    virtual FText GetTooltipTextForCache() const { return FText::GetEmpty(); };

private:
    /* Constructing FText strings can be costly, so we cache the node's title */
    FNodeTextTable CachedTexts;
};
