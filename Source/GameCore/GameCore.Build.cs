// Copyright Freebooz Games, Inc. All Rights Reserved.
// GameCore - 基础层模块构建配置

using UnrealBuildTool;

public class GameCore : ModuleRules
{
	public GameCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// 公共依赖模块
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"GameplayTags",
			"InputCore",
			"UMG",
		});

		// 私有依赖模块
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore",
			"HTTP",
			"Json",
			"JsonUtilities",
		});

		// 公共 Include 路径
		PublicIncludePaths.AddRange(new string[]
		{
			"GameCore/Public",
		});

		// 私有 Include 路径
		PrivateIncludePaths.AddRange(new string[]
		{
			"GameCore/Private",
		});

		bUseUnity = true;
		bLegacyPublicIncludePaths = false;

		CppStandard = CppStandardVersion.Cpp20;
	}
}
