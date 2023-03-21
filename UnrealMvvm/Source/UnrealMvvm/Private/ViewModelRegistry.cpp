// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Mvvm/Impl/ViewModelRegistry.h"
#include "Mvvm/BaseViewModel.h"
#include "Mvvm/Impl/ViewModelPropertyIterator.h"
#include "Algo/Compare.h"

namespace UnrealMvvm_Impl
{

#if WITH_EDITOR
FViewModelRegistry::FViewModelClassChanged FViewModelRegistry::ViewClassChanged;
#endif
TMap<UClass*, TArray<FViewModelPropertyReflection>> FViewModelRegistry::ViewModelProperties{};
TMap<UClass*, UClass*> FViewModelRegistry::ViewModelClasses{};
TMap<UClass*, FViewModelRegistry::FViewModelSetterPtr> FViewModelRegistry::ViewModelSetters{};
TArray<FViewModelRegistry::FUnprocessedPropertyEntry> FViewModelRegistry::UnprocessedProperties{};
TArray<FViewModelRegistry::FUnprocessedViewModelClassEntry> FViewModelRegistry::UnprocessedViewModelClasses{};

template <typename TValue>
TValue* FindByClass(TMap<UClass*, TValue*>& Map, UClass* ViewClass)
{
    UClass* Needle = ViewClass;

    while (Needle)
    {
        TValue** FoundPtr = Map.Find(Needle);
        if (FoundPtr)
        {
            return *FoundPtr;
        }

        Needle = Needle->GetSuperClass();
    }

    return nullptr;
}

const FViewModelPropertyReflection* FViewModelRegistry::FindProperty(UClass* InViewModelClass, const FName& InPropertyName)
{
    if (InViewModelClass)
    {
        return FindPropertyInternal(InViewModelClass, InPropertyName);
    }

    return nullptr;
}

UClass* FViewModelRegistry::GetViewModelClass(UClass* ViewClass)
{
    return FindByClass(ViewModelClasses, ViewClass);
}

FViewModelRegistry::FViewModelSetterPtr FViewModelRegistry::GetViewModelSetter(UClass* ViewClass)
{
    return FindByClass(ViewModelSetters, ViewClass);
}

uint8 FViewModelRegistry::RegisterViewClass(FViewModelRegistry::FClassGetterPtr ViewClassGetter, FViewModelRegistry::FClassGetterPtr ViewModelClassGetter, FViewModelRegistry::FViewModelSetterPtr ViewModelSetter)
{
    FUnprocessedViewModelClassEntry& Entry = UnprocessedViewModelClasses.AddDefaulted_GetRef();
    Entry.GetViewClass = ViewClassGetter;
    Entry.GetViewModelClass = ViewModelClassGetter;
    Entry.ViewModelSetter = ViewModelSetter;

    return 1;
}

void FViewModelRegistry::RegisterViewClass(UClass* ViewClass, UClass* ViewModelClass)
{
    check(ViewClass);
    check(ViewModelClass);

    ViewModelClasses.Emplace(ViewClass, ViewModelClass);
#if WITH_EDITOR
    ViewClassChanged.Broadcast(ViewClass, ViewModelClass);
#endif
}

void FViewModelRegistry::ProcessPendingRegistrations()
{
    if (GIsInitialLoad)
    {
        // wait until UObject subsystem is loaded
        return;
    }

    // Process properties and add them into lookup tables
    TArray<UClass*> NewlyAddedViewModels;

    if (UnprocessedProperties.Num())
    {
        for (auto& Property : UnprocessedProperties)
        {
            UClass* NewClass = Property.GetClass();

            TArray<FViewModelPropertyReflection>* NewArray = ViewModelProperties.Find(NewClass);
            if (!NewArray)
            {
                NewArray = &ViewModelProperties.Emplace(NewClass);
                NewlyAddedViewModels.Emplace(NewClass);
            }

            NewArray->Add(Property.Reflection);
        }

        UnprocessedProperties.Empty();
    }

    // Patch all new ViewModel classes, so their properties will be processed by GC
    for (UClass* ViewModelClass : NewlyAddedViewModels)
    {
        GenerateReferenceTokenStream(ViewModelClass);
    }

    // Process classes and add them into lookup tables
    if (UnprocessedViewModelClasses.Num())
    {
        for (auto& Entry : UnprocessedViewModelClasses)
        {
            UClass* ViewClass = Entry.GetViewClass();
            UClass* ViewModelClass = Entry.GetViewModelClass();

            ViewModelClasses.Add(ViewClass, ViewModelClass);

            if (Entry.ViewModelSetter)
            {
                ViewModelSetters.Add(ViewClass, Entry.ViewModelSetter);
            }

#if WITH_EDITOR
            ViewClassChanged.Broadcast(ViewClass, ViewModelClass);
#endif
        }

        UnprocessedViewModelClasses.Empty();
    }
}

const FViewModelPropertyReflection* FViewModelRegistry::FindPropertyInternal(UClass* InViewModelClass, const FName& InPropertyName)
{
    // find properties of requested class
    TArray<FViewModelPropertyReflection>* ArrayPtr = ViewModelProperties.Find(InViewModelClass);

    if (ArrayPtr)
    {
        for (const FViewModelPropertyReflection& Item : *ArrayPtr)
        {
            if (Item.GetProperty()->GetName() == InPropertyName)
            {
                return &Item;
            }
        }
    }

    // if not found - look in a super class
    UClass* SuperClass = InViewModelClass->GetSuperClass();
    if (SuperClass && SuperClass->IsChildOf<UBaseViewModel>())
    {
        return FindPropertyInternal(SuperClass, InPropertyName);
    }

    return nullptr;
}

void FViewModelRegistry::GenerateReferenceTokenStream(UClass* ViewModelClass)
{
    UClass* TempClass = NewObject<UClass>((UObject*)GetTransientPackage(), FName(), EObjectFlags::RF_Transient);
#if ENGINE_MAJOR_VERSION >= 5
    TempClass->CppClassStaticFunctions.SetAddReferencedObjects(&UObject::AddReferencedObjects);
#else
    TempClass->ClassAddReferencedObjects = &UObject::AddReferencedObjects;
#endif

    // Create FProperty objects and add them to TempClass
    for (FViewModelPropertyIterator Iter(ViewModelClass, false); Iter; ++Iter)
    {
        Iter->GetOperations().AddClassProperty(TempClass);
    }

    // Finalize Class and sssemble ReferenceTokenStream
    TempClass->StaticLink();
    TempClass->AssembleReferenceTokenStream();

    // Transfer properties from TempClass to ViewModelClass if needed
    if (TransferTokenStream(TempClass, ViewModelClass))
    {
        // Pull out Map and Set properties and store them separately
        // We need to do this, because TempClass will delete all properties that it owns
        // TMap and TSet properties are used during garbage collection, so we must keep them alive
        TArray<FField*> Props = ExtractMapAndSetProperties(TempClass);

        // TODO: Keep Props somewhere and destroy them when time comes
        // Maybe listen to ViewModelClass destruction or hot reload to perform cleanup 
    }
}

bool FViewModelRegistry::TransferTokenStream(UClass* Source, UClass* Destination)
{
    // Helper class that mirrors layout of FGCReferenceTokenStream
    // We cannot use its fields directly because they are private :(

#if ENGINE_MAJOR_VERSION >= 5
    struct FReferenceTokenStreamDouble
    {
        TArray<uint32> Tokens;
        int32 StackSize;
        EGCTokenType TokenType;
#if ENABLE_GC_OBJECT_CHECKS
        TArray<FName> TokenDebugInfo;
#endif
    };
#else
    struct FReferenceTokenStreamDouble
    {
        TArray<uint32> Tokens;
#if ENABLE_GC_OBJECT_CHECKS
        TArray<FName> TokenDebugInfo;
#endif
    };
#endif

    FReferenceTokenStreamDouble& SourceStream = reinterpret_cast<FReferenceTokenStreamDouble&>(Source->ReferenceTokenStream);
    FReferenceTokenStreamDouble& DestinationStream = reinterpret_cast<FReferenceTokenStreamDouble&>(Destination->ReferenceTokenStream);

    // Find which tokens to take from Source Stream
    const uint32 OuterToken = 0x20300;

    int32 LastSourceTokenIdx = SourceStream.Tokens.IndexOfByKey(OuterToken);
    check(LastSourceTokenIdx != -1);

    // Find where to insert tokens into DestinationStream
    int32 LastDestinationTokenIdx = DestinationStream.Tokens.IndexOfByKey(OuterToken);
    check(LastDestinationTokenIdx != -1);

    // Insert tokens if Source has any
    if (LastSourceTokenIdx > 0)
    {
        bool bWasPatched = false;

        // Compare DestinationStream with SourceStream to determine if it was already patched
        if (LastDestinationTokenIdx > LastSourceTokenIdx)
        {
            bWasPatched = Algo::Compare(
                MakeArrayView(SourceStream.Tokens.GetData(), LastSourceTokenIdx),
                MakeArrayView(DestinationStream.Tokens.GetData() + LastDestinationTokenIdx - LastSourceTokenIdx, LastDestinationTokenIdx)
            );
        }

        // Insert only if we have not already
        if (!bWasPatched)
        {
            const int32 NewDestinationNum = DestinationStream.Tokens.Num() + LastSourceTokenIdx;
            DestinationStream.Tokens.Reserve(NewDestinationNum);
            DestinationStream.Tokens.Insert(SourceStream.Tokens.GetData(), LastSourceTokenIdx, LastDestinationTokenIdx);

#if ENGINE_MAJOR_VERSION >= 5
            DestinationStream.StackSize = FMath::Max(DestinationStream.StackSize, SourceStream.StackSize);
#endif

#if ENABLE_GC_OBJECT_CHECKS
            DestinationStream.TokenDebugInfo.Reserve(NewDestinationNum);
            DestinationStream.TokenDebugInfo.Insert(SourceStream.TokenDebugInfo.GetData(), LastSourceTokenIdx, LastDestinationTokenIdx);
#endif

            return true;
        }
    }

    return false;
}

TArray<FField*> FViewModelRegistry::ExtractMapAndSetProperties(UClass* InClass)
{
    TArray<FField*> Result;

    FField* PrevField = nullptr;
    FField* CurrentField = InClass->ChildProperties;

    while (CurrentField)
    {
        FField* NextField = CurrentField->Next;

        if (CurrentField->IsA<FMapProperty>() || CurrentField->IsA<FSetProperty>())
        {
            Result.Add(CurrentField);

            if (PrevField)
            {
                PrevField->Next = NextField;
            }
            else
            {
                InClass->ChildProperties = NextField;
            }
        }
        else
        {
            PrevField = CurrentField;
        }

        CurrentField = NextField;
    }

    return Result;
}

}