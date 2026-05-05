// Copyright Freebooz Games, Inc. All Rights Reserved.
// 怪物AI状态枚举

#pragma once

#include "CoreMinimal.h"
#include "EMonsterAIState.generated.h"

/**
 * EMonsterAIState
 * 怪物AI状态枚举
 */
UENUM(BlueprintType)
enum class EMonsterAIState : uint8
{
	Idle    UMETA(DisplayName = "Idle", Tooltip = "原地待机状态"),
	Patrol  UMETA(DisplayName = "Patrol", Tooltip = "巡逻状态"),
	Chase   UMETA(DisplayName = "Chase", Tooltip = "追击状态"),
	Attack  UMETA(DisplayName = "Attack", Tooltip = "攻击状态"),
	Return  UMETA(DisplayName = "Return", Tooltip = "返回状态")
};