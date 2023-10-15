// Copyright Andrei Sudarikov. All Rights Reserved.

#include "MacrosTestViewModel.h"

#define VM_PROP_ACCESSOR_TEST_COMMON_DEFINE(Suffix, ...) \
void  UMacrosTestViewModel::SetPropAgMs ## Suffix (int32 NewValue) { PropAgMs ## Suffix ## Field = NewValue; } \
int32 UMacrosTestViewModel::GetPropMgAs ## Suffix () const { return PropMgAs ## Suffix ## Field; } \
int32 UMacrosTestViewModel::GetPropMgMs ## Suffix () const { return PropMgMs ## Suffix ## Field; } \
void  UMacrosTestViewModel::SetPropMgMs ## Suffix (int32 NewValue) { PropMgMs ## Suffix ## Field = NewValue; } \
void  UMacrosTestViewModel::SetPropAgMsNf ## Suffix (int32 NewValue) { PropAgMsNf ## Suffix ## Field = NewValue; } \
int32 UMacrosTestViewModel::GetPropMgAsNf ## Suffix () const { return PropMgAsNf ## Suffix ## Field; } \
int32 UMacrosTestViewModel::GetPropMgMsNf ## Suffix () const { return PropMgMsNf ## Suffix ## Field; } \
void  UMacrosTestViewModel::SetPropMgMsNf ## Suffix (int32 NewValue) { PropMgMsNf ## Suffix ## Field = NewValue; } \

void UMacrosTestViewModel::SetPropAgMs(int32 NewValue) { PropAgMsField = NewValue; }

int32 UMacrosTestViewModel::GetPropMgAs() const { return PropMgAsField; }

int32 UMacrosTestViewModel::GetPropMgMs() const { return PropMgMsField; }
void UMacrosTestViewModel::SetPropMgMs(int32 NewValue) { PropMgMsField = NewValue; }

void UMacrosTestViewModel::SetPropAgMsNf(int32 NewValue) { PropAgMsNfField = NewValue; }
int32 UMacrosTestViewModel::GetPropMgAsNf() const { return PropMgAsNfField; }

int32 UMacrosTestViewModel::GetPropMgMsNf() const { return PropMgMsNfField; }
void UMacrosTestViewModel::SetPropMgMsNf(int32 NewValue) { PropMgMsNfField = NewValue; }

int32 UMacrosTestViewModel::GetPropMgNf() const { return PropMgNfField; }

VM_PROP_ACCESSOR_TEST_COMMON_DEFINE(PubPub, public, public);
VM_PROP_ACCESSOR_TEST_COMMON_DEFINE(PubPriv, public, private);
VM_PROP_ACCESSOR_TEST_COMMON_DEFINE(PrivPub, private, public);
VM_PROP_ACCESSOR_TEST_COMMON_DEFINE(PrivPriv, private, private);

VM_PROP_ACCESSOR_TEST_COMMON_DEFINE(Pub, public);
VM_PROP_ACCESSOR_TEST_COMMON_DEFINE(Priv, private);

VM_PROP_ACCESSOR_TEST_COMMON_DEFINE(Def);

#undef VM_PROP_ACCESSOR_TEST_COMMON_DEFINE