// Copyright Epic Games, Inc. All Rights Reserved.
// 神兽竞技场 - Editor Target 配置

using UnrealBuildTool;
using System.Collections.Generic;

public class DivineBeastsArenaEditorTarget : TargetRules
{
    public DivineBeastsArenaEditorTarget(TargetInfo Target) : base(Target)
    {
        // Target 类型：编辑器
        Type = TargetType.Editor;

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
            // 编辑器标识
            "DBA_EDITOR=1",
            "DBA_CLIENT=1",
            "DBA_SERVER=0",
        });

        // 编译设置
        bUseLoggingInShipping = true;               // Editor 始终启用日志
        bCompileWithAccessibilitySupport = true;    // 启用无障碍支持
        bCompileAgainstEngine = true;               // 编译引擎代码
        bCompileAgainstCoreUObject = true;          // 编译 CoreUObject
        bBuildDeveloperTools = true;                 // 构建开发者工具
        bBuildWithEditorOnlyData = true;            // 包含 Editor 专用数据

        // 性能优化
        bUseSharedPCHs = true;                      // 使用共享 PCH

        // Editor 特定设置
        bBuildTargetDeveloperTools = true;          // 构建目标开发者工具
        bCompileICU = true;                         // 编译 ICU（国际化组件）

        // 日志输出
        System.Console.WriteLine("DivineBeastsArena Editor Target: Building for " + Target.Platform + " (" + Target.Configuration + ")");
    }
}