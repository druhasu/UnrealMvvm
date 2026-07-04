// Copyright Andrei Sudarikov. All Rights Reserved.

using UnrealBuildTool;

public class UnrealMvvmTestsEditor : ModuleRules
{
    public UnrealMvvmTestsEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
        [
            "Core",
            "UMG",
        ]);

        PrivateDependencyModuleNames.AddRange(
        [
            "BlueprintGraph",
            "CoreUObject",
            "Engine",
            "Slate",
            "SlateCore",
            "UnrealEd",
            "UnrealMvvmTests",
            "UnrealMvvmEditor",
        ]);

        UnrealMvvm.Setup(this);
    }
}
