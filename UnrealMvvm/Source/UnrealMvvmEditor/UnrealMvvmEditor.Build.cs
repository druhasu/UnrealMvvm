// Copyright Andrei Sudarikov. All Rights Reserved.

using UnrealBuildTool;

public class UnrealMvvmEditor : ModuleRules
{
	public UnrealMvvmEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new []
			{
				"Core",
				"UMG",
				// ... add other public dependencies that you statically link with here ...
			});
		
		PrivateDependencyModuleNames.AddRange(
			new []
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"PropertyEditor",
				"EditorStyle",
				"BlueprintGraph",
				"GraphEditor",
				"KismetCompiler",
				"UnrealEd",
				"UnrealMvvm",
				// ... add private dependencies that you statically link with here ...
			});
	}
}
