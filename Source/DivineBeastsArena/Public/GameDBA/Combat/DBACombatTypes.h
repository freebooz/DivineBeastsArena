// Copyright Freebooz Games, Inc. All Rights Reserved.
// 战斗系统类型定义

#pragma once

#include "CoreMinimal.h"
#include "DBACombatTypes.generated.h"

/**
 * EDBATargetLockStrategy
 * 目标锁定策略枚举
 */
UENUM(BlueprintType)
enum class EDBATargetLockStrategy : uint8
{
	Smart      UMETA(DisplayName = "Smart", Tooltip = "智能锁定（威胁最高）"),
	LowestHP   UMETA(DisplayName = "LowestHP", Tooltip = "血量锁定（最低血量）"),
	Nearest    UMETA(DisplayName = "Nearest", Tooltip = "距离锁定（最近目标）")
};

/**
 * EDADeathState
 * 角色死亡状态枚举
 */
UENUM(BlueprintType)
enum class EDADeathState : uint8
{
	Alive       UMETA(DisplayName = "Alive", Tooltip = "存活状态"),
	Dying       UMETA(DisplayName = "Dying", Tooltip = "死亡中（播放死亡动画）"),
	Dead        UMETA(DisplayName = "Dead", Tooltip = "死亡状态"),
	Respawning  UMETA(DisplayName = "Respawning", Tooltip = "复活中")
};