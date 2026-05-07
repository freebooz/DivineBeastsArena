// Copyright Freebooz Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GameMoba : ModuleRules
{
	public GameMoba(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			"UMG",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"GameCore",
		});

		PublicIncludePaths.Add("GameMoba/Public");
		PrivateIncludePaths.Add("GameMoba/Private");

		bUseUnity = true;
		CppStandard = CppStandardVersion.Cpp20;
	}
}
