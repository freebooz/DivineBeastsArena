// Copyright Epic Games, Inc. All Rights Reserved.
// 神兽竞技场 - MOBA GameState 基类

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Common/Types/DBACommonEnums.h"
#include "DBAGameStateBase.generated.h"

/**
 * ADBAMobaGameStateBase
 * MOBA Base 层 GameState 基类
 * 提供通用 MOBA 竞技状态管理
 * 不绑定项目专属内容
 */
UCLASS(Abstract, Blueprintable)
class DIVINEBEASTSARENA_API ADBAMobaGameStateBase : public AGameStateBase
{
    GENERATED_BODY()

public:
    ADBAMobaGameStateBase();

protected:
    // 游戏开始时调用
    virtual void BeginPlay() override;

public:
    // 获取已过去的时间
    UFUNCTION(BlueprintCallable, Category = "GameState")
    float GetElapsedTime() const;

    // 获取游戏时长限制（0 表示无限制）
    UFUNCTION(BlueprintCallable, Category = "GameState")
    float GetMatchDurationLimit() const;

    // 获取剩余时间
    UFUNCTION(BlueprintCallable, Category = "GameState")
    float GetRemainingTime() const;

    // 获取当前队伍数量
    UFUNCTION(BlueprintCallable, Category = "GameState")
    int32 GetTeamCount() const;

    // 获取某队伍的玩家数量
    UFUNCTION(BlueprintCallable, Category = "GameState")
    int32 GetPlayerCountByTeam(int32 TeamId) const;

    // 获取所有玩家状态列表
    UFUNCTION(BlueprintCallable, Category = "GameState")
    TArray<APlayerState*> GetAllPlayerStates() const;

protected:
    // 游戏时长限制（秒，0 表示无限制）
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameState")
    float MatchDurationLimit = 0.0f;

    // 当前游戏状态
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "GameState")
    EDBAGameModeState CurrentGameState = EDBAGameModeState::None;
};