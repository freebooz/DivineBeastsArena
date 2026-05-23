/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：应用源码文件，承担该模块的一部分业务、界面、配置或启动逻辑。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

using UnrealBuildTool;

public class DivineBeastsArena : ModuleRules
{
    public DivineBeastsArena(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "GameplayAbilities",
            "GameplayTags",
            "GameplayTasks",
            "UMG",
            "GameCore",
            "GameMoba",
            "Niagara",
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Slate",
            "SlateCore",
            "NetCore",
            "OnlineSubsystem",
            "OnlineSubsystemUtils",
            "DeveloperSettings",
            "AssetRegistry",
            "AIModule",
            "NavigationSystem",
            "Json",
            "JsonUtilities",
            "GameBackendClient",
        });

        if (Target.Type != TargetType.Server)
        {
            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "RenderCore",
                "RHI",
                "AudioMixer",
            });
        }

        PrivateDependencyModuleNames.Add("MediaAssets");

        if (Target.Type == TargetType.Editor)
        {
            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "UnrealEd",
                "EditorSubsystem",
            });
        }

        if (Target.Configuration != UnrealTargetConfiguration.Shipping)
        {
            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "HTTP",
                "Json",
                "JsonUtilities",
            });
        }

        PublicIncludePaths.AddRange(new string[]
        {
            "DivineBeastsArena/Public",
        });

        PrivateIncludePaths.AddRange(new string[]
        {
            "DivineBeastsArena/Private",
        });

        PublicDefinitions.AddRange(new string[]
        {
            "ENABLE_GAS_DEBUG=" + (Target.Configuration != UnrealTargetConfiguration.Shipping ? "1" : "0"),
            "ENABLE_NETWORK_DEBUG=" + (Target.Configuration != UnrealTargetConfiguration.Shipping ? "1" : "0"),
            "ENABLE_EXTERNAL_SERVICES=" + (Target.Configuration != UnrealTargetConfiguration.Shipping ? "1" : "0"),
        });

        bUseUnity = true;
        bLegacyPublicIncludePaths = false;
        CppStandard = CppStandardVersion.Cpp20;

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicSystemLibraries.Add("Winmm.lib");
        }
    }
}
