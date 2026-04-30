// Copyright Freebooz Games, Inc. All Rights Reserved.
// 神兽竞技场 - 主模块实现

#include "DivineBeastsArena.h"
#include "Modules/ModuleManager.h"

// 日志分类定义（后续在 DBALog.h 中详细定义）
DEFINE_LOG_CATEGORY_STATIC(LogDBA, Log, All);

void FDivineBeastsArenaModule::StartupModule()
{
    UE_LOG(LogDBA, Log, TEXT("[DivineBeastsArena] 模块启动"));

    // 初始化日志分类
    InitializeLogging();

    // 注册资产类型
    RegisterAssetTypes();

    // 初始化 GameplayTag
    InitializeGameplayTags();

    // Dedicated Server 特定初始化
#if UE_SERVER
    UE_LOG(LogDBA, Log, TEXT("[DivineBeastsArena] 运行在 Dedicated Server 模式"));
#endif

    // Client 特定初始化
#if !UE_SERVER
    UE_LOG(LogDBA, Log, TEXT("[DivineBeastsArena] 运行在 Client 模式"));
#endif

    // Editor 特定初始化
#if WITH_EDITOR
    UE_LOG(LogDBA, Log, TEXT("[DivineBeastsArena] 运行在 Editor 模式"));
#endif
}

void FDivineBeastsArenaModule::ShutdownModule()
{
    UE_LOG(LogDBA, Log, TEXT("[DivineBeastsArena] 模块关闭"));

    // 清理资源
    CleanupResources();
}

void FDivineBeastsArenaModule::InitializeLogging()
{
    // 日志初始化（后续在第 8 部分详细实现）
    UE_LOG(LogDBA, Log, TEXT("[DivineBeastsArena] 初始化日志系统"));
}

void FDivineBeastsArenaModule::RegisterAssetTypes()
{
    // 资产类型注册（后续在第 9 部分详细实现）
    UE_LOG(LogDBA, Log, TEXT("[DivineBeastsArena] 注册资产类型"));
}

void FDivineBeastsArenaModule::InitializeGameplayTags()
{
    // GameplayTag 初始化（后续在第 7 部分详细实现）
    UE_LOG(LogDBA, Log, TEXT("[DivineBeastsArena] 初始化 GameplayTags"));
}

void FDivineBeastsArenaModule::CleanupResources()
{
    // 资源清理
    UE_LOG(LogDBA, Log, TEXT("[DivineBeastsArena] 清理资源"));
}

// 实现模块接口
IMPLEMENT_PRIMARY_GAME_MODULE(FDivineBeastsArenaModule, DivineBeastsArena, "DivineBeastsArena");