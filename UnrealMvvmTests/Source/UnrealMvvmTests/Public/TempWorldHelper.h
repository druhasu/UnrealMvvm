// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Engine/World.h"

struct FTempWorldHelper
{
    FTempWorldHelper()
    {
        World = UWorld::CreateWorld(EWorldType::PIE, false);
    }

    ~FTempWorldHelper()
    {
        World->DestroyWorld(false);
    }

    UWorld* World;
};
