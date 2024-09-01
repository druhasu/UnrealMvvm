// Copyright Andrei Sudarikov. All Rights Reserved.

#include "Modules/ModuleManager.h"
#include "Mvvm/Impl/BaseView/ViewRegistry.h"
#include "Mvvm/Impl/Property/ViewModelRegistry.h"

class FUnrealMvvmModuleImpl : public IModuleInterface
{
public:
    void StartupModule() override
    {
        FModuleManager::Get().OnModulesChanged().AddRaw(this, &FUnrealMvvmModuleImpl::OnModulesChanged);
        UnrealMvvm_Impl::FViewModelRegistry::ProcessPendingRegistrations();
        UnrealMvvm_Impl::FViewRegistry::ProcessPendingRegistrations();
    }

    void ShutdownModule() override
    {
        FModuleManager::Get().OnModulesChanged().RemoveAll(this);
        UnrealMvvm_Impl::FViewModelRegistry::DeleteKeptProperties();
    }

private:
    void OnModulesChanged(FName InModule, EModuleChangeReason InReason)
    {
        if (InReason == EModuleChangeReason::ModuleLoaded)
        {
            UnrealMvvm_Impl::FViewModelRegistry::ProcessPendingRegistrations();
            UnrealMvvm_Impl::FViewRegistry::ProcessPendingRegistrations();
        }
    }
};

IMPLEMENT_MODULE(FUnrealMvvmModuleImpl, UnrealMvvm)
