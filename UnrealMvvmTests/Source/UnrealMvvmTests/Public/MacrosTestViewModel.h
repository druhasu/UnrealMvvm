// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/BaseViewModel.h"
#include "MacrosTestViewModel.generated.h"

/*
* This ViewModel uses all avalable macros for defining properties and serves as a compile test for them
*/
UCLASS()
class UMacrosTestViewModel : public UBaseViewModel
{
    GENERATED_BODY()

    VM_PROP_AG_AS(int32, PropAgAs, public, public);
    VM_PROP_AG_MS(int32, PropAgMs, public, public);
    VM_PROP_MG_AS(int32, PropMgAs, public, public);
    VM_PROP_MG_MS(int32, PropMgMs, public, public);

    VM_PROP_AG_AS_NF(int32, PropAgAsNf, public, public);
    VM_PROP_AG_MS_NF(int32, PropAgMsNf, public, public);
    VM_PROP_MG_AS_NF(int32, PropMgAsNf, public, public);
    VM_PROP_MG_MS_NF(int32, PropMgMsNf, public, public);

    VM_PROP_MG_NF(int32, PropMgNf, public);

    VM_PROP_AG_AS(const int32&, RefPropAgAs, public, public);
    VM_PROP_AG_AS(int32*, PtrPropAgAs, public, public);

private:
    int32 PropAgAsNfField = 0;
    int32 PropAgMsNfField = 0;
    int32 PropMgAsNfField = 0;
    int32 PropMgMsNfField = 0;

    int32 PropMgNfField = 42;
};