// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

/*#include "Mvvm/ViewModelGCWrapper.h"
#include "UObject/Object.h"
#include "UObject/GCObject.h"
#include "UObject/StrongObjectPtr.h"
#include "Templates/UnrealTypeTraits.h"*/

template <typename T>
struct TPropertyTypeSelector
{
    using GetterType = T;
    using SetterType = const T&;
};

template <typename T>
struct TPropertyTypeSelector<T*>
{
    using GetterType = T*;
    using SetterType = T*;
};