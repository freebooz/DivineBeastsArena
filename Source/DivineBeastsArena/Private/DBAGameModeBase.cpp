// Copyright Epic Games, Inc. All Rights Reserved.
// 神兽竞技场 - 基础 GameMode 实现

#include "DBAGameModeBase.h"
#include "Common/DBALogChannels.h"

// 构造函数 - 初始化游戏模式基础配置
ADBAGameModeBase::ADBAGameModeBase()
{
    // 基础配置
}

// BeginPlay - 游戏开始时调用
void ADBAGameModeBase::BeginPlay()
{
    Super::BeginPlay();

    // 验证日志
    UE_LOG(LogDBACore, Log, TEXT("DBAGameModeBase: BeginPlay - 神兽竞技场已启动"));

    // Dedicated Server 检测
    if (GetNetMode() == NM_DedicatedServer)
    {
        UE_LOG(LogDBACore, Log, TEXT("DBAGameModeBase: 运行在 Dedicated Server 模式"));
    }
}