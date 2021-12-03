// Copyright Andrei Sudarikov. All Rights Reserved.

#include "MacrosTestViewModel.h"

void UMacrosTestViewModel::SetPropAgMs(int32 NewValue) { PropAgMsField = NewValue; }

int32 UMacrosTestViewModel::GetPropMgAs() const { return PropMgAsField; }

int32 UMacrosTestViewModel::GetPropMgMs() const { return PropMgMsField; }
void UMacrosTestViewModel::SetPropMgMs(int32 NewValue) { PropMgMsField = NewValue; }

void UMacrosTestViewModel::SetPropAgMsNf(int32 NewValue) { PropAgMsNfField = NewValue; }
int32 UMacrosTestViewModel::GetPropMgAsNf() const { return PropMgAsNfField; }

int32 UMacrosTestViewModel::GetPropMgMsNf() const { return PropMgMsNfField; }
void UMacrosTestViewModel::SetPropMgMsNf(int32 NewValue) { PropMgMsNfField = NewValue; }

int32 UMacrosTestViewModel::GetPropMgNf() const { return PropMgNfField; }