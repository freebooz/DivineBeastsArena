// Copyright Epic Games, Inc. All Rights Reserved.
// 神兽竞技场 - 主模块构建配置

using UnrealBuildTool;

public class DivineBeastsArena : ModuleRules
{
    public DivineBeastsArena(ReadOnlyTargetRules Target) : base(Target)
    {
        // PCH 使用策略
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // 公共依赖模块（在 Public 头文件中使用，需要暴露给其他模块）
        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",                     // 核心模块
            "CoreUObject",              // UObject 系统
            "Engine",                   // 引擎核心
            "InputCore",                // 输入核心
            "EnhancedInput",            // 增强输入系统
            "GameplayAbilities",        // GAS 核心
            "GameplayTags",             // GameplayTag 系统
            "GameplayTasks",            // GameplayTask 系统
            "UMG",                      // UI 系统
        });

        // 私有依赖模块（仅在 Private 实现中使用，不暴露给其他模块）
        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Slate",                    // Slate UI 框架
            "SlateCore",                // Slate 核心
            "NetCore",                  // 网络核心
            "OnlineSubsystem",          // 在线子系统（用于 Session 管理）
            "OnlineSubsystemUtils",     // 在线子系统工具
            "DeveloperSettings",        // 开发者设置
            "AssetRegistry",            // 资产注册表
            "AIModule",                 // AI 模块（用于 TeamAgentInterface）
        });

        // Dedicated Server 不需要的模块（客户端专用）
        if (Target.Type != TargetType.Server)
        {
            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "RenderCore",           // 渲染核心
                "RHI",                  // 渲染硬件接口
                "Niagara",              // Niagara 特效系统
                "AudioMixer",           // 音频混音器
            });
        }

        // MediaAssets 模块（启动视频需要）
        // 注意：即使在服务器构建中链接此模块，视频功能也不会被执行（ShouldCreateSubsystem 返回 false）
        PrivateDependencyModuleNames.Add("MediaAssets");

        // Editor 专用模块
        if (Target.Type == TargetType.Editor)
        {
            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "UnrealEd",             // 编辑器核心
                "EditorSubsystem",      // 编辑器子系统
            });
        }

        // 可选的外部服务客户端依赖（用于 Monitoring / GameOps）
        // 这些依赖仅用于可选功能，游戏运行不强制依赖
        if (Target.Configuration != UnrealTargetConfiguration.Shipping)
        {
            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "HTTP",                 // HTTP 请求（可选监控上报）
                "Json",                 // JSON 解析
                "JsonUtilities",        // JSON 工具
            });
        }

        // 公共 Include 路径
        PublicIncludePaths.AddRange(new string[]
        {
            "DivineBeastsArena/Public",
        });

        // 私有 Include 路径
        PrivateIncludePaths.AddRange(new string[]
        {
            "DivineBeastsArena/Private",
            "DivineBeastsArena/Internal",
        });

        // 预编译宏定义
        PublicDefinitions.AddRange(new string[]
        {
            // 启用 GAS 调试（非 Shipping 构建）
            "ENABLE_GAS_DEBUG=" + (Target.Configuration != UnrealTargetConfiguration.Shipping ? "1" : "0"),

            // 启用网络调试（非 Shipping 构建）
            "ENABLE_NETWORK_DEBUG=" + (Target.Configuration != UnrealTargetConfiguration.Shipping ? "1" : "0"),

            // 启用外部服务客户端（非 Shipping 构建）
            "ENABLE_EXTERNAL_SERVICES=" + (Target.Configuration != UnrealTargetConfiguration.Shipping ? "1" : "0"),
        });

        // 优化设置
        bUseUnity = true;                           // 启用 Unity 构建加速编译
        bLegacyPublicIncludePaths = false;          // 禁用旧版公共 Include 路径

        // C++ 标准
        CppStandard = CppStandardVersion.Cpp20;

        // 日志输出
        System.Console.WriteLine("DivineBeastsArena Module: Building for " + Target.Type + " (" + Target.Configuration + ")");
    }
}