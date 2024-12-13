// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/BaseViewModel.h"
#include "MacrosTestViewModel.generated.h"

#define VM_PROP_ACCESSOR_TEST_COMMON_DECLARE_FIELDS(Suffix) \
    int32 PropAgAsNf ## Suffix ## Field = 0; \
    int32 PropAgMsNf ## Suffix ## Field = 0; \
    int32 PropMgAsNf ## Suffix ## Field = 0; \
    int32 PropMgMsNf ## Suffix ## Field = 0;

/*
 * Empty struct to force multiple inheritance in UMacrosTestViewModel
 */
struct FMacrosTestMixin {};

/*
* This ViewModel uses all avalable macros for defining properties and serves as a compile test for them
*/
UCLASS()
class UMacrosTestViewModel : public UBaseViewModel, public FMacrosTestMixin
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
    VM_PROP_MG_NF(int32, PropMgNfRet, public) { return 42; }

    VM_PROP_AG_AS(const int32&, RefPropAgAs, public, public);
    VM_PROP_AG_AS(int32*, PtrPropAgAs, public, public);

    // public, public
    VM_PROP_AG_AS(int32, PropAgAsPubPub, public, public);
    VM_PROP_AG_MS(int32, PropAgMsPubPub, public, public);
    VM_PROP_MG_AS(int32, PropMgAsPubPub, public, public);
    VM_PROP_MG_MS(int32, PropMgMsPubPub, public, public);
    VM_PROP_AG_AS_NF(int32, PropAgAsNfPubPub, public, public);
    VM_PROP_AG_MS_NF(int32, PropAgMsNfPubPub, public, public);
    VM_PROP_MG_AS_NF(int32, PropMgAsNfPubPub, public, public);
    VM_PROP_MG_MS_NF(int32, PropMgMsNfPubPub, public, public);
    VM_PROP_ACCESSOR_TEST_COMMON_DECLARE_FIELDS(PubPub);

    // public, private
    VM_PROP_AG_AS(int32, PropAgAsPubPriv, public, private);
    VM_PROP_AG_MS(int32, PropAgMsPubPriv, public, private);
    VM_PROP_MG_AS(int32, PropMgAsPubPriv, public, private);
    VM_PROP_MG_MS(int32, PropMgMsPubPriv, public, private);
    VM_PROP_AG_AS_NF(int32, PropAgAsNfPubPriv, public, private);
    VM_PROP_AG_MS_NF(int32, PropAgMsNfPubPriv, public, private);
    VM_PROP_MG_AS_NF(int32, PropMgAsNfPubPriv, public, private);
    VM_PROP_MG_MS_NF(int32, PropMgMsNfPubPriv, public, private);
    VM_PROP_ACCESSOR_TEST_COMMON_DECLARE_FIELDS(PubPriv);

    // private, public
    VM_PROP_AG_AS(int32, PropAgAsPrivPub, private, public);
    VM_PROP_AG_MS(int32, PropAgMsPrivPub, private, public);
    VM_PROP_MG_AS(int32, PropMgAsPrivPub, private, public);
    VM_PROP_MG_MS(int32, PropMgMsPrivPub, private, public);
    VM_PROP_AG_AS_NF(int32, PropAgAsNfPrivPub, private, public);
    VM_PROP_AG_MS_NF(int32, PropAgMsNfPrivPub, private, public);
    VM_PROP_MG_AS_NF(int32, PropMgAsNfPrivPub, private, public);
    VM_PROP_MG_MS_NF(int32, PropMgMsNfPrivPub, private, public);
    VM_PROP_ACCESSOR_TEST_COMMON_DECLARE_FIELDS(PrivPub);

    // private, private
    VM_PROP_AG_AS(int32, PropAgAsPrivPriv, private, private);
    VM_PROP_AG_MS(int32, PropAgMsPrivPriv, private, private);
    VM_PROP_MG_AS(int32, PropMgAsPrivPriv, private, private);
    VM_PROP_MG_MS(int32, PropMgMsPrivPriv, private, private);
    VM_PROP_AG_AS_NF(int32, PropAgAsNfPrivPriv, private, private);
    VM_PROP_AG_MS_NF(int32, PropAgMsNfPrivPriv, private, private);
    VM_PROP_MG_AS_NF(int32, PropMgAsNfPrivPriv, private, private);
    VM_PROP_MG_MS_NF(int32, PropMgMsNfPrivPriv, private, private);
    VM_PROP_ACCESSOR_TEST_COMMON_DECLARE_FIELDS(PrivPriv);

    // public
    VM_PROP_AG_AS(int32, PropAgAsPub, public);
    VM_PROP_AG_MS(int32, PropAgMsPub, public);
    VM_PROP_MG_AS(int32, PropMgAsPub, public);
    VM_PROP_MG_MS(int32, PropMgMsPub, public);
    VM_PROP_AG_AS_NF(int32, PropAgAsNfPub, public);
    VM_PROP_AG_MS_NF(int32, PropAgMsNfPub, public);
    VM_PROP_MG_AS_NF(int32, PropMgAsNfPub, public);
    VM_PROP_MG_MS_NF(int32, PropMgMsNfPub, public);
    VM_PROP_ACCESSOR_TEST_COMMON_DECLARE_FIELDS(Pub);

    // private
    VM_PROP_AG_AS(int32, PropAgAsPriv, private);
    VM_PROP_AG_MS(int32, PropAgMsPriv, private);
    VM_PROP_MG_AS(int32, PropMgAsPriv, private);
    VM_PROP_MG_MS(int32, PropMgMsPriv, private);
    VM_PROP_AG_AS_NF(int32, PropAgAsNfPriv, private);
    VM_PROP_AG_MS_NF(int32, PropAgMsNfPriv, private);
    VM_PROP_MG_AS_NF(int32, PropMgAsNfPriv, private);
    VM_PROP_MG_MS_NF(int32, PropMgMsNfPriv, private);
    VM_PROP_ACCESSOR_TEST_COMMON_DECLARE_FIELDS(Priv);

    // empty
    VM_PROP_AG_AS(int32, PropAgAsDef);
    VM_PROP_AG_MS(int32, PropAgMsDef);
    VM_PROP_MG_AS(int32, PropMgAsDef);
    VM_PROP_MG_MS(int32, PropMgMsDef);
    VM_PROP_AG_AS_NF(int32, PropAgAsNfDef);
    VM_PROP_AG_MS_NF(int32, PropAgMsNfDef);
    VM_PROP_MG_AS_NF(int32, PropMgAsNfDef);
    VM_PROP_MG_MS_NF(int32, PropMgMsNfDef);
    VM_PROP_ACCESSOR_TEST_COMMON_DECLARE_FIELDS(Def);

    VM_PROP_MG_NF(int32, PropMgNfPub, public) { return 42; }
    VM_PROP_MG_NF(int32, PropMgNfPriv, private) { return 42; }
    VM_PROP_MG_NF(int32, PropMgNfDef) { return 42; }

private:
    int32 PropAgAsNfField = 0;
    int32 PropAgMsNfField = 0;
    int32 PropMgAsNfField = 0;
    int32 PropMgMsNfField = 0;

    int32 PropMgNfField = 42;
};

#undef VM_PROP_ACCESSOR_TEST_COMMON_DECLARE_FIELDS
