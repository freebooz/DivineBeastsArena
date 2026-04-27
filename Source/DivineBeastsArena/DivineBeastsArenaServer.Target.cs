// Copyright Epic Games, Inc. All Rights Reserved.
// 神兽竞技场 - Dedicated Server Target 配置

using UnrealBuildTool;
using System.Collections.Generic;

public class DivineBeastsArenaServerTarget : TargetRules
{
	public DivineBeastsArenaServerTarget(TargetInfo Target) : base(Target)
	{
		// Target 类型：服务器
		Type = TargetType.Server;

		// 默认构建设置版本
		DefaultBuildSettings = BuildSettingsVersion.V6;

		// Include 顺序版本
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		// 额外模块名称
		ExtraModuleNames.Add("DivineBeastsArena");

		// 全局定义
		GlobalDefinitions.AddRange(new string[]
		{
			// 服务器标识
			"DBA_CLIENT=0",
			"DBA_SERVER=1",
		});

		// 服务器构建设置
		bUseLoggingInShipping = true;               	// 服务器启用日志
		bCompileWithAccessibilitySupport = false;    	// 服务器禁用无障碍支持
		bCompileAgainstEngine = true;                	// 编译引擎代码
		bCompileAgainstCoreUObject = true;           	// 编译 CoreUObject
		bBuildDeveloperTools = false;                	// 服务器不构建开发者工具
		bBuildWithEditorOnlyData = false;          		// 不包含 Editor 专用数据

		// 性能优化
		bUseSharedPCHs = true;                     		// 使用共享 PCH
		OptimizationLevel = OptimizationMode.Speed;  	// 优化速度

		// 平台特定设置
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			GlobalDefinitions.Add("DBA_WINDOWS=1");
		}
		else if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			GlobalDefinitions.Add("DBA_LINUX=1");
		}

		// 日志输出
		System.Console.WriteLine("DivineBeastsArena Server Target: Building for " + Target.Platform + " (" + Target.Configuration + ")");
	}
}
