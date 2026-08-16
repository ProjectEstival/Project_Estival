// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProjectEstival : ModuleRules
{
	public ProjectEstival(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			//Core default
			"Core",
			"CoreUObject",
			"Engine",
			
			//New modules
			"AssetTools",
			
			//Other
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"NavigationSystem",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"Niagara",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"ProjectEstival",
			"ProjectEstival/Variant_Strategy",
			"ProjectEstival/Variant_Strategy/UI",
			"ProjectEstival/Variant_TwinStick",
			"ProjectEstival/Variant_TwinStick/AI",
			"ProjectEstival/Variant_TwinStick/Gameplay",
			"ProjectEstival/Variant_TwinStick/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
