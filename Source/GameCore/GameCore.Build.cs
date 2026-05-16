// Copyright Freebooz Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GameCore : ModuleRules
{
	public GameCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"GameplayTags",
			"InputCore",
			"UMG",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore",
			"HTTP",
			"Json",
			"JsonUtilities",
			"GameBackendClient",
		});

		PublicIncludePaths.Add("GameCore/Public");
		PrivateIncludePaths.Add("GameCore/Private");

		bUseUnity = true;
		CppStandard = CppStandardVersion.Cpp20;
	}
}
