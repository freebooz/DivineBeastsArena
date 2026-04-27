// Copyright Epic Games, Inc. All Rights Reserved.
// 神兽竞技场 - Server Target 配置

using UnrealBuildTool;
using System.Collections.Generic;

public class DivineBeastsArenaServerTarget : TargetRules
{
	public DivineBeastsArenaServerTarget(TargetInfo Target) : base(Target)
	{
		// Target 类型：服务器
		Type = TargetType.Server;

		// 构建环境
		BuildEnvironment = TargetBuildEnvironment.Unique;

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
			"DBA_SERVER=1",
			"DBA_CLIENT=0",
		});

		// 编译设置
		bUseLoggingInShipping = true;               // 服务器始终启用日志
		bCompileWithAccessibilitySupport = false;   // 服务器不需要无障碍支持
		bCompileAgainstEngine = true;               // 编译引擎代码
		bCompileAgainstCoreUObject = true;          // 编译 CoreUObject
		bBuildDeveloperTools = false;               // 服务器不需要开发者工具
		bBuildWithEditorOnlyData = false;          // 不包含 Editor 专用数据

		// 性能优化
		bUseSharedPCHs = true;                      // 使用共享 PCH

		// 日志输出
		System.Console.WriteLine("DivineBeastsArena Server Target: Building for " + Target.Platform + " (" + Target.Configuration + ")");
	}
}