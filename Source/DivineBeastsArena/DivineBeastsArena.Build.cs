// Copyright Freebooz Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class DivineBeastsArena : ModuleRules
{
	public DivineBeastsArena(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			"UMG",
			"GameCore",
			"GameMoba",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore",
			"NetCore",
			"OnlineSubsystem",
			"OnlineSubsystemUtils",
			"DeveloperSettings",
			"AssetRegistry",
			"AIModule",
			"NavigationSystem",
			"MediaAssets",
		});

		if (Target.Type != TargetType.Server)
		{
			PrivateDependencyModuleNames.AddRange(new string[]
			{
				"RenderCore",
				"RHI",
				"Niagara",
				"AudioMixer",
			});
		}

		if (Target.Type == TargetType.Editor)
		{
			PrivateDependencyModuleNames.AddRange(new string[]
			{
				"UnrealEd",
				"EditorSubsystem",
			});
		}

		if (Target.Configuration != UnrealTargetConfiguration.Shipping)
		{
			PrivateDependencyModuleNames.AddRange(new string[]
			{
				"HTTP",
				"Json",
				"JsonUtilities",
			});
		}

		string GameCorePublic = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "GameCore", "Public")).Replace('\\', '/');
		string GameMobaPublic = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "GameMoba", "Public")).Replace('\\', '/');

		PublicIncludePaths.AddRange(new string[]
		{
			Path.Combine(ModuleDirectory, "Public").Replace('\\', '/'),
			Path.Combine(ModuleDirectory, "Public", "GameDBA").Replace('\\', '/'),
			GameCorePublic,
			GameMobaPublic,
		});

		PrivateIncludePaths.AddRange(new string[]
		{
			Path.Combine(ModuleDirectory, "Private").Replace('\\', '/'),
			Path.Combine(ModuleDirectory, "Private", "GameDBA").Replace('\\', '/'),
		});

		PublicDefinitions.AddRange(new string[]
		{
			"ENABLE_GAS_DEBUG=" + (Target.Configuration != UnrealTargetConfiguration.Shipping ? "1" : "0"),
			"ENABLE_NETWORK_DEBUG=" + (Target.Configuration != UnrealTargetConfiguration.Shipping ? "1" : "0"),
			"ENABLE_EXTERNAL_SERVICES=" + (Target.Configuration != UnrealTargetConfiguration.Shipping ? "1" : "0"),
		});

		bUseUnity = true;
		CppStandard = CppStandardVersion.Cpp20;

		System.Console.WriteLine("DivineBeastsArena Module: Building for " + Target.Type + " (" + Target.Configuration + ")");
	}
}

