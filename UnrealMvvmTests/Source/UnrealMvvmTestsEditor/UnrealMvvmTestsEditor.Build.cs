// Copyright Andrei Sudarikov. All Rights Reserved.

using UnrealBuildTool;

public class UnrealMvvmTestsEditor : ModuleRules
{
    public UnrealMvvmTestsEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "UMG",
                "UnrealMvvm",
            });

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "BlueprintGraph",
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "UnrealEd",
                "UnrealMvvmTests",
                "UnrealMvvmEditor",
            });
    }
}
