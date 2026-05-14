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
        });

        if (Target.Type != TargetType.Server)
        {
            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "RenderCore",
                "RHI",
                "Niagara",
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

