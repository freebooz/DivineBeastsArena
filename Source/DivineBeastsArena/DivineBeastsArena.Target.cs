// Copyright Epic Games, Inc. All Rights Reserved.
// 神兽竞技场 - Client Target 配置

using UnrealBuildTool;
using System.Collections.Generic;

public class DivineBeastsArenaTarget : TargetRules
{
	public DivineBeastsArenaTarget(TargetInfo Target) : base(Target)
	{
		// Target 类型：客户端
		Type = TargetType.Game;

		// 默认构建设置版本
		DefaultBuildSettings = BuildSettingsVersion.V6;

		// Include 顺序版本
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		// 额外模块名称
		ExtraModuleNames.Add("DivineBeastsArena");

		// 全局定义
		GlobalDefinitions.AddRange(new string[]
		{
            // 客户端标识
            "DBA_CLIENT=1",
			"DBA_SERVER=0",
		});

		// 编译设置
		bUseLoggingInShipping = false;              // Shipping 构建禁用日志
		bCompileWithAccessibilitySupport = true;    // 启用无障碍支持
		bCompileAgainstEngine = true;               // 编译引擎代码
		bCompileAgainstCoreUObject = true;         // 编译 CoreUObject
		bBuildDeveloperTools = true;                // 构建开发者工具
		bBuildWithEditorOnlyData = false;           // 不包含 Editor 专用数据

		// 性能优化
		bUseSharedPCHs = true;                      // 使用共享 PCH

		// 平台特定设置
		if (Target.Platform == UnrealTargetPlatform.Android)
		{
			// Android 特定设置
			GlobalDefinitions.Add("DBA_ANDROID=1");

			// Android 性能优化 - 优化包体大小
			OptimizationLevel = OptimizationMode.Size;
		}
		else if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			// Windows 特定设置
			GlobalDefinitions.Add("DBA_WINDOWS=1");
		}
		else if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			// Linux 特定设置
			GlobalDefinitions.Add("DBA_LINUX=1");
		}

		// 日志输出
		System.Console.WriteLine("DivineBeastsArena Client Target: Building for " + Target.Platform + " (" + Target.Configuration + ")");
	}
}