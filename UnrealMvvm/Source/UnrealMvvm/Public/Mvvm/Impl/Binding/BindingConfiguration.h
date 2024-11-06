// Copyright Andrei Sudarikov. All Rights Reserved.

#pragma once

#include "Mvvm/ViewModelProperty.h"
#include "Mvvm/Impl/Binding/IPropertyChangeHandler.h"
#include "Containers/ArrayView.h"
#include "Templates/TypeCompatibleBytes.h"
#include "Algo/AllOf.h"

class UBaseViewModel;
struct FNativeHandlerBinding;
struct FBlueprintBindingEntry;

namespace UnrealMvvm_Impl
{

    struct FResolvedViewModelEntry
    {
        // union here because we never need both values.
        // Class is only needed in "template" version and ViewModel is only needed in "instanced" version
        union
        {
            UClass* ViewModelClass;
            UBaseViewModel* ViewModel;
        };

        int32 FirstProperty;
        int32 NumProperties;

        friend bool operator== (const FResolvedViewModelEntry& Entry, UBaseViewModel* InViewModel)
        {
            return Entry.ViewModel == InViewModel;
        }
    };

    struct FResolvedPropertyEntry
    {
        FResolvedPropertyEntry(const FViewModelPropertyBase* InProperty, int8 InNextViewModelIndex)
            : Property(InProperty)
            , NextViewModelIndex(InNextViewModelIndex)
            , bHasHandler(false)
            , bInline(true)
            , HandlerSize(0)
            , HandlerBuffer()
        {
        }

        ~FResolvedPropertyEntry()
        {
            if (bHasHandler)
            {
                IPropertyChangeHandler* Handler = GetHandler();
                bHasHandler = false;
                Handler->~IPropertyChangeHandler();

                if (!bInline)
                {
                    FMemory::Free(*((void**)&HandlerBuffer));
                }
            }
        }

        template <typename THandler, typename... TArgs>
        void EmplaceHandler(TArgs&&... Args)
        {
            check(!bHasHandler);
            constexpr bool bInlineConstexpr = sizeof(THandler) <= HandlerBufferSize;

            bHasHandler = true;
            bInline = bInlineConstexpr;
            HandlerSize = sizeof(THandler); // for analytics

            if constexpr (!bInlineConstexpr)
            {
                THandler* AllocatedMemory = (THandler*)FMemory::Malloc(sizeof(THandler));
                *((THandler**)&HandlerBuffer) = AllocatedMemory;
            }

            new(GetHandler()) THandler(Forward<TArgs>(Args)...);
        }

        IPropertyChangeHandler* GetHandler() const
        {
            if (bHasHandler)
            {
                return bInline
                    ? ((IPropertyChangeHandler*)&HandlerBuffer)
                    : *((IPropertyChangeHandler**)&HandlerBuffer);
            }

            return nullptr;
        }

        friend bool operator== (const FResolvedPropertyEntry& Entry, const FViewModelPropertyBase* InProperty)
        {
            return Entry.Property == InProperty;
        }

        friend bool operator== (const FResolvedPropertyEntry& Entry, FName PropertyName)
        {
            return Entry.Property->GetName() == PropertyName;
        }

        static constexpr int32 HandlerBufferSize = sizeof(void*) * 4;

        const FViewModelPropertyBase* Property;
        int8 NextViewModelIndex;
        bool bHasHandler;
        bool bInline;
        int8 HandlerSize;

        TAlignedBytes<HandlerBufferSize, 8> HandlerBuffer;
    };

    struct FBindingConfiguration
    {
        struct FHeader
        {
            // total size of allocation
            int16 Size;

            // offset to beginning of properties array
            int16 PropertiesOffset;

            // number of ViewModel entries
            uint8 NumViewModels;

            // number of Property entries
            uint8 NumProperties;

            // whether owning BindingWorker has subscribed to root ViewModel
            // we store the flag here to save 8 bytes of memory inside BindingWorker
            bool bHasSubscription;
        };

        FBindingConfiguration() : Data(nullptr) {}
        FBindingConfiguration(const FBindingConfiguration& Other)
        {
            *this = Other;
        }

        FBindingConfiguration(FBindingConfiguration&& Other)
        {
            *this = MoveTemp(Other);
        }

        FBindingConfiguration(uint8 NumViewModels, uint8 NumProperties)
        {
            const int32 PropertiesOffset = ViewModelsOffset + sizeof(FResolvedViewModelEntry) * NumViewModels;
            int32 DataSize = PropertiesOffset + sizeof(FResolvedPropertyEntry) * NumProperties;

            Data = (uint8*)FMemory::Malloc(DataSize);

            FHeader* Header = (FHeader*)Data;
            Header->Size = DataSize;
            Header->PropertiesOffset = PropertiesOffset;
            Header->NumViewModels = NumViewModels;
            Header->NumProperties = NumProperties;
            Header->bHasSubscription = false;
        }

        FBindingConfiguration& operator= (const FBindingConfiguration& Other)
        {
            if (Data != nullptr)
            {
                FMemory::Free(Data);
            }

            if (Other.Data != nullptr)
            {
                checkSlow(VerifyEmptyHandlers(Other));

                const FHeader* Header = (const FHeader*)Other.Data;

                Data = (uint8*)FMemory::Malloc(Header->Size);
                FMemory::Memcpy(Data, Other.Data, Header->Size);
            }
            else
            {
                Data = nullptr;
            }

            return *this;
        }

        FBindingConfiguration& operator= (FBindingConfiguration&& Other)
        {
            Data = Other.Data;
            Other.Data = nullptr;

            return *this;
        }

        ~FBindingConfiguration()
        {
            if (Data != nullptr)
            {
                for (FResolvedPropertyEntry& PropertyEntry : GetProperties())
                {
                    // make sure property entries are destructed, because they may allocate additional memory
                    PropertyEntry.~FResolvedPropertyEntry();
                }

                FMemory::Free(Data);
            }
        }

        TArrayView<FResolvedViewModelEntry> GetViewModels()
        {
            if (Data != nullptr)
            {
                return MakeArrayView((FResolvedViewModelEntry*)(Data + ViewModelsOffset), GetHeader()->NumViewModels);
            }
            return {};
        }

        TArrayView<FResolvedPropertyEntry> GetProperties()
        {
            if (Data != nullptr)
            {
                return MakeArrayView((FResolvedPropertyEntry*)(Data + GetHeader()->PropertiesOffset), GetHeader()->NumProperties);
            }
            return {};
        }

        TArrayView<FResolvedPropertyEntry> GetProperties(const FResolvedViewModelEntry& ViewModelEntry)
        {
            if (Data != nullptr)
            {
                return MakeArrayView((FResolvedPropertyEntry*)(Data + GetHeader()->PropertiesOffset) + ViewModelEntry.FirstProperty, ViewModelEntry.NumProperties);
            }
            return {};
        }

        FHeader* GetHeader()
        {
            return (FHeader*)Data;
        }

        const FHeader* GetHeader() const
        {
            return (const FHeader*)Data;
        }

        bool HasSubscription() const
        {
            return Data != nullptr ? GetHeader()->bHasSubscription : false;
        }

        void SetHasSubscription(bool bValue)
        {
            if (Data)
            {
                GetHeader()->bHasSubscription = bValue;
            }
        }

        static constexpr int32 ViewModelsOffset = 8;
        uint8* Data;

    private:
        bool VerifyEmptyHandlers(const FBindingConfiguration& Other)
        {
            TArrayView<FResolvedPropertyEntry> PropertyEntries = const_cast<FBindingConfiguration&>(Other).GetProperties();

            return Algo::AllOf(PropertyEntries, [](const FResolvedPropertyEntry& Entry) { return !Entry.bHasHandler; });
        }
    };

}
