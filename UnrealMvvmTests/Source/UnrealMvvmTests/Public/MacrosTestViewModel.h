// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/BaseViewModel.h"
#include "MacrosTestViewModel.generated.h"

#define VM_PROP_ACCESSOR_TEST_COMMON_DECLARE(Suffix, ...) \
    VM_PROP_AG_AS(int32, PropAgAs ## Suffix, ##__VA_ARGS__); \
    VM_PROP_AG_MS(int32, PropAgMs ## Suffix, ##__VA_ARGS__); \
    VM_PROP_MG_AS(int32, PropMgAs ## Suffix, ##__VA_ARGS__); \
    VM_PROP_MG_MS(int32, PropMgMs ## Suffix, ##__VA_ARGS__); \
    VM_PROP_AG_AS_NF(int32, PropAgAsNf ## Suffix, ##__VA_ARGS__); \
    VM_PROP_AG_MS_NF(int32, PropAgMsNf ## Suffix, ##__VA_ARGS__); \
    VM_PROP_MG_AS_NF(int32, PropMgAsNf ## Suffix, ##__VA_ARGS__); \
    VM_PROP_MG_MS_NF(int32, PropMgMsNf ## Suffix, ##__VA_ARGS__); \
    int32 PropAgAsNf ## Suffix ## Field = 0; \
    int32 PropAgMsNf ## Suffix ## Field = 0; \
    int32 PropMgAsNf ## Suffix ## Field = 0; \
    int32 PropMgMsNf ## Suffix ## Field = 0;

#define VM_PROP_ACCESSOR_TEST_GETTER_ONLY_DECLARE(Suffix, ...) \
    VM_PROP_MG_NF(int32, PropMgNf ## Suffix, ##__VA_ARGS__) { return 42; }

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

    VM_PROP_ACCESSOR_TEST_COMMON_DECLARE(PubPub, public, public);
    VM_PROP_ACCESSOR_TEST_COMMON_DECLARE(PubPriv, public, private);
    VM_PROP_ACCESSOR_TEST_COMMON_DECLARE(PrivPub, private, public);
    VM_PROP_ACCESSOR_TEST_COMMON_DECLARE(PrivPriv, private, private);

    VM_PROP_ACCESSOR_TEST_COMMON_DECLARE(Pub, public);
    VM_PROP_ACCESSOR_TEST_COMMON_DECLARE(Priv, private);

    VM_PROP_ACCESSOR_TEST_COMMON_DECLARE(Def);

    VM_PROP_ACCESSOR_TEST_GETTER_ONLY_DECLARE(Pub, public);
    VM_PROP_ACCESSOR_TEST_GETTER_ONLY_DECLARE(Priv, private);

    VM_PROP_ACCESSOR_TEST_GETTER_ONLY_DECLARE(Def);

private:
    int32 PropAgAsNfField = 0;
    int32 PropAgMsNfField = 0;
    int32 PropMgAsNfField = 0;
    int32 PropMgMsNfField = 0;

    int32 PropMgNfField = 42;
};

#undef VM_PROP_ACCESSOR_TEST_COMMON_DECLARE
#undef VM_PROP_ACCESSOR_TEST_GETTER_ONLY_DECLARE