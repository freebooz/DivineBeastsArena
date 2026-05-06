// Copyright Freebooz Games, Inc. All Rights Reserved.
// GameMoba - MOBA逻辑层模块构建配置

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

		// 公共 Include 路径
		PublicIncludePaths.AddRange(new string[]
		{
			"GameMoba/Public",
		});

		// 私有 Include 路径
		PrivateIncludePaths.AddRange(new string[]
		{
			"GameMoba/Private",
		});

		bUseUnity = true;
		bLegacyPublicIncludePaths = false;

		CppStandard = CppStandardVersion.Cpp20;
	}
}