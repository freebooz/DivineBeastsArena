// Copyright Freebooz Games, Inc. All Rights Reserved.
// GameCore - 基础层模块构建配置

using System.IO;
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
		});

		// 私有依赖模块
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore",
		});

		// 公共 Include 路径 - 使用 UE5 标准的相对路径格式
		PublicIncludePaths.Add("GameCore/Public");

		// 私有 Include 路径
		PrivateIncludePaths.Add("GameCore/Private");

		// API 宏定义
		PublicDefinitions.Add("GAMECORE_API=");

		bUseUnity = true;
		// bLegacyPublicIncludePaths = false; // 使用 UE5 默认值

		CppStandard = CppStandardVersion.Cpp20;
	}
}