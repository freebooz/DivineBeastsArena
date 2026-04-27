// Copyright Epic Games, Inc. All Rights Reserved.
// 神兽竞技场 - Dedicated Server Target 配置

using UnrealBuildTool;
using System.Collections.Generic;

public class DivineBeastsArenaServerTarget : TargetRules
{
	public DivineBeastsArenaServerTarget(TargetInfo Target) : base(Target)
	{
		// Target 类型：Dedicated Server
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
            // Dedicated Server 标识
            "DBA_SERVER=1",
			"DBA_CLIENT=0",
			"DBA_DEDICATED_SERVER=1",
		});

		// Dedicated Server 优化配置
		bUseLoggingInShipping = true;               // Server 始终启用日志
		bCompileWithAccessibilitySupport = false;   // Server 不需要无障碍支持
		bCompileAgainstEngine = true;               // 编译引擎代码
		bCompileAgainstCoreUObject = true;          // 编译 CoreUObject

		// 禁用客户端相关功能
		bBuildDeveloperTools = false;               // 不构建开发者工具
		bBuildWithEditorOnlyData = false;           // 不包含 Editor 专用数据

		// 性能优化
		bUseSharedPCHs = true;                      // 使用共享 PCH
		OptimizationLevel = OptimizationMode.Speed; // 优先性能而非大小

		// Server 特定优化
		bWithServerCode = true;                     // 包含服务器代码

		// 禁用客户端渲染相关
		bCompileRecast = true;                      // 编译 Recast 导航（AI 需要）

		// 平台特定设置
		if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			// Linux Dedicated Server 特定设置
			GlobalDefinitions.Add("DBA_LINUX_SERVER=1");
		}
		else if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			// Windows Dedicated Server 特定设置
			GlobalDefinitions.Add("DBA_WINDOWS_SERVER=1");
		}

		// 日志输出
		System.Console.WriteLine("DivineBeastsArena Dedicated Server Target: Building for " + Target.Platform + " (" + Target.Configuration + ")");
	}
}