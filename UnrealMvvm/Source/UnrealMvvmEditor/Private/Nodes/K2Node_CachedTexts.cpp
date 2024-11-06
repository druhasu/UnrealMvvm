// Copyright Andrei Sudarikov. All Rights Reserved.

#include "K2Node_CachedTexts.h"

FText UK2Node_CachedTexts::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
    if (!CachedTexts.IsTitleCached(TitleType, this))
    {
        CachedTexts.SetCachedTitle(TitleType, GetNodeTitleForCache(TitleType), this);
    }

    return CachedTexts.GetCachedTitle(TitleType);
}

FText UK2Node_CachedTexts::GetTooltipText() const
{
    if (!CachedTexts.IsTooltipCached(this))
    {
        CachedTexts.SetCachedTooltip(GetTooltipTextForCache(), this);
    }

    return CachedTexts.GetCachedTooltip();
}

bool UK2Node_CachedTexts::Modify(bool bAlwaysMarkDirty)
{
    CachedTexts.MarkDirty();

    return Super::Modify(bAlwaysMarkDirty);
}
