// Copyright Andrei Sudarikov. All Rights Reserved.

using UnrealBuildTool;

public class UnrealMvvmTests : ModuleRules
{
    public UnrealMvvmTests(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
        [
            "Core",
            "UMG",
            // ... add other public dependencies that you statically link with here ...
        ]);

        PrivateDependencyModuleNames.AddRange(
        [
            "CoreUObject",
            "Engine",
            "Slate",
            "SlateCore",
            // ... add private dependencies that you statically link with here ...	
        ]);

        if (Target.Type == TargetRules.TargetType.Editor)
        {
            PrivateDependencyModuleNames.AddRange(
            [
                "BlueprintGraph",
                "UnrealMvvmEditor",
            ]);
        }

        UnrealMvvm.Setup(this);
    }
}
