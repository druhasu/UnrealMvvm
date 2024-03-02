// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Mvvm/BaseViewModel.h"
#include "Containers/Map.h"

/*
 * Helper struct that counts how many times each property changed
 */
struct FPropertyChangeCounter
{
    FPropertyChangeCounter(UBaseViewModel* InViewModel)
    {
        ViewModel = InViewModel;
        ViewModel->Subscribe(UBaseViewModel::FPropertyChangedDelegate::FDelegate::CreateRaw(this, &FPropertyChangeCounter::OnPropertyChanged));
    }

    ~FPropertyChangeCounter()
    {
        ViewModel->Unsubscribe(this);
    }

    int32 operator[] (const FViewModelPropertyBase* Prop) const
    {
        return Changes.FindRef(Prop);
    }

    void OnPropertyChanged(const FViewModelPropertyBase* Prop)
    {
        Changes.FindOrAdd(Prop) += 1;
    }

    TMap<const FViewModelPropertyBase*, int32> Changes;
    UBaseViewModel* ViewModel;
};
