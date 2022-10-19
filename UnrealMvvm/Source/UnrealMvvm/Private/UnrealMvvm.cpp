// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Modules/ModuleManager.h"
#include "Mvvm/Impl/ViewModelRegistry.h"

class FUnrealMvvmModuleImpl : public IModuleInterface
{
public:
    void StartupModule() override
    {
        FModuleManager::Get().OnModulesChanged().AddRaw(this, &FUnrealMvvmModuleImpl::RegisterViewModelClasses);
        UnrealMvvm_Impl::FViewModelRegistry::ProcessPendingRegistrations();
    }

    void ShutdownModule() override
    {
        FModuleManager::Get().OnModulesChanged().RemoveAll(this);
    }

private:
    void RegisterViewModelClasses(FName InModule, EModuleChangeReason InReason)
    {
        if (InReason == EModuleChangeReason::ModuleLoaded)
        {
            UnrealMvvm_Impl::FViewModelRegistry::ProcessPendingRegistrations();
        }
    }
};

IMPLEMENT_MODULE(FUnrealMvvmModuleImpl, UnrealMvvm)