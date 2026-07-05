// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// 神兽竞技场 - 主模块实现

#include "DivineBeastsArena.h"
#include "GameDBA/Core/DBAGameplayTags.h"
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

    // 专用服务器特定初始化
#if UE_SERVER
    UE_LOG(LogDBA, Log, TEXT("[DivineBeastsArena] 运行在专用服务器模式"));
#endif

    // 客户端特定初始化
#if !UE_SERVER
    UE_LOG(LogDBA, Log, TEXT("[DivineBeastsArena] 运行在客户端模式"));
#endif

    // 编辑器特定初始化
#if WITH_EDITOR
    UE_LOG(LogDBA, Log, TEXT("[DivineBeastsArena] 运行在编辑器模式"));
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
    FDBAGameplayTags::InitializeNativeTags();
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
