// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

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


        // 默认构建设置版本
        DefaultBuildSettings = BuildSettingsVersion.V7;

        // Include 顺序版本
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

        // 额外模块名称
        ExtraModuleNames.Add("DivineBeastsArena");
        ExtraModuleNames.Add("GameCore");
        ExtraModuleNames.Add("GameMoba");

        // 编译设置
        bUseLoggingInShipping = false;
        bOverrideBuildEnvironment = false;              // 使用引擎默认 Editor 构建环境，避免 DLL 导入宏与预编译引擎不一致
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
