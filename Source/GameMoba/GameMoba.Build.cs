// Copyright Freebooz Games, Inc. All Rights Reserved.
// GameMoba - MOBA逻辑层模块构建配置

using System.IO;
using UnrealBuildTool;

public class GameMoba : ModuleRules
{
	public GameMoba(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// 公共依赖模块
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
		});

		// 私有依赖模块
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"GameCore",               // GameMoba 依赖 GameCore
		});

		// 公共 Include 路径 - 使用 UE5 标准的相对路径格式
		PublicIncludePaths.Add("GameMoba/Public");

		// 私有 Include 路径
		PrivateIncludePaths.Add("GameMoba/Private");

		// API 宏定义
		PublicDefinitions.Add("GAMEMOBA_API=");

		bUseUnity = true;
		// bLegacyPublicIncludePaths = false; // 使用 UE5 默认值

		CppStandard = CppStandardVersion.Cpp20;
	}
}