// Copyright Andrei Sudarikov. All Rights Reserved.

#include "MacrosTestViewModel.h"

void UMacrosTestViewModel::SetPropAgMs(const int32& NewValue) { PropAgMsField = NewValue; }

int32 UMacrosTestViewModel::GetPropMgAs() { return PropMgAsField; }

int32 UMacrosTestViewModel::GetPropMgMs() { return PropMgMsField; }
void UMacrosTestViewModel::SetPropMgMs(const int32& NewValue) { PropMgMsField = NewValue; }

void UMacrosTestViewModel::SetPropAgMsNf(const int32& NewValue) { PropAgMsNfField = NewValue; }
int32 UMacrosTestViewModel::GetPropMgAsNf() { return PropMgAsNfField; }

int32 UMacrosTestViewModel::GetPropMgMsNf() { return PropMgMsNfField; }
void UMacrosTestViewModel::SetPropMgMsNf(const int32& NewValue) { PropMgMsNfField = NewValue; }

int32 UMacrosTestViewModel::GetPropMgNf() { return PropMgNfField; }