// Copyright Freebooz Games, Inc. All Rights Reserved.
// 神兽竞技场 - MOBA GameMode 基类

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "Common/Types/DBACommonEnums.h"
#include "DBAMobaGameModeBase.generated.h"

/**
 * ADBAMobaGameModeBase
 * MOBA Base 层 GameMode 基类
 * 提供通用 MOBA 竞技逻辑框架
 * 不绑定项目专属内容（十二生肖、自然元素之力、五大阵营）
 */
UCLASS(Abstract, Blueprintable)
class DIVINEBEASTSARENA_API ADBAMobaGameModeBase : public AGameModeBase
{
    GENERATED_BODY()

public:
    ADBAMobaGameModeBase();

protected:
    // 游戏开始时调用
    virtual void BeginPlay() override;

    // 游戏结束时的回调
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    // 客户端连接时的回调
    virtual void PostLogin(APlayerController* NewPlayer) override;

    // 客户端断开时的回调
    virtual void Logout(AController* Exiting) override;

    // 处理游戏暂停
    virtual bool Pause();

    // 恢复游戏暂停
    virtual bool UnPause();

public:
    // 获取游戏是否在运行
    UFUNCTION(BlueprintCallable, Category = "GameMode")
    bool IsGameRunning() const;

    // 获取当前玩家数量
    UFUNCTION(BlueprintCallable, Category = "GameMode")
    int32 GetCurrentPlayerCount() const;

    // 获取最大玩家数量
    UFUNCTION(BlueprintCallable, Category = "GameMode")
    int32 GetMaxPlayerCount() const;

protected:
    // 玩家数量改变时的回调（子类重写）
    UFUNCTION(BlueprintNativeEvent, Category = "GameMode")
    void OnPlayerCountChanged(int32 OldCount, int32 NewCount);

    // 游戏状态改变时的回调（子类重写）
    UFUNCTION(BlueprintNativeEvent, Category = "GameMode")
    void OnGameStateChanged(EDBAGameModeState NewState);
};