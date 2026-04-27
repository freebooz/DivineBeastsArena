// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MobaBase/UI/UDBAMobaWidgetControllerBase.h"
#include "UDBAMobaHUDWidgetControllerBase.generated.h"

/**
 * MOBA 游戏 HUD Widget 控制器基类
 *
 * 提供 MOBA 游戏 HUD 通用功能：
 * - 玩家生命值/魔法值显示
 * - 经验值/金币显示
 * - 复活时间显示
 * - 击杀/死亡/助攻统计
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAMobaHUDWidgetControllerBase : public UDBAMobaWidgetControllerBase
{
	GENERATED_BODY()

public:
	UDBAMobaHUDWidgetControllerBase(const FObjectInitializer& ObjectInitializer);

	/**
	 * 更新玩家生命值
	 */
	UFUNCTION(BlueprintCallable, Category = "MobaBase|UI|HUD")
	virtual void UpdatePlayerHP(float CurrentHP, float MaxHP);

	/**
	 * 更新玩家魔法值/能量
	 */
	UFUNCTION(BlueprintCallable, Category = "MobaBase|UI|HUD")
	virtual void UpdatePlayerMana(float CurrentMana, float MaxMana);

	/**
	 * 更新玩家经验值
	 */
	UFUNCTION(BlueprintCallable, Category = "MobaBase|UI|HUD")
	virtual void UpdatePlayerXP(float CurrentXP, float XPGain);

	/**
	 * 更新玩家金币
	 */
	UFUNCTION(BlueprintCallable, Category = "MobaBase|UI|HUD")
	virtual void UpdatePlayerGold(int32 CurrentGold);

	/**
	 * 更新击杀/死亡/助攻
	 */
	UFUNCTION(BlueprintCallable, Category = "MobaBase|UI|HUD")
	virtual void UpdatePlayerStats(int32 Kills, int32 Deaths, int32 Assists);

	/**
	 * 更新复活时间
	 */
	UFUNCTION(BlueprintCallable, Category = "MobaBase|UI|HUD")
	virtual void UpdateRespawnTime(float RemainingTime);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerHPChanged, float, CurrentHP, float, MaxHP);
	UPROPERTY(BlueprintAssignable, Category = "MobaBase|UI|HUD")
	FOnPlayerHPChanged OnPlayerHPChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerManaChanged, float, CurrentMana, float, MaxMana);
	UPROPERTY(BlueprintAssignable, Category = "MobaBase|UI|HUD")
	FOnPlayerManaChanged OnPlayerManaChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerXPChanged, float, CurrentXP);
	UPROPERTY(BlueprintAssignable, Category = "MobaBase|UI|HUD")
	FOnPlayerXPChanged OnPlayerXPChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerGoldChanged, int32, CurrentGold);
	UPROPERTY(BlueprintAssignable, Category = "MobaBase|UI|HUD")
	FOnPlayerGoldChanged OnPlayerGoldChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPlayerStatsChanged, int32, Kills, int32, Deaths, int32, Assists);
	UPROPERTY(BlueprintAssignable, Category = "MobaBase|UI|HUD")
	FOnPlayerStatsChanged OnPlayerStatsChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRespawnTimeChanged, float, RemainingTime);
	UPROPERTY(BlueprintAssignable, Category = "MobaBase|UI|HUD")
	FOnRespawnTimeChanged OnRespawnTimeChanged;
};
