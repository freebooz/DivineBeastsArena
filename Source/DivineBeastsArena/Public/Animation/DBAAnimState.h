// Copyright Freebooz Games, Inc. All Rights Reserved.
// 动画状态枚举

#pragma once

#include "CoreMinimal.h"
#include "DBAAnimState.generated.h"

/**
 * EDBAAnimState
 * 动画状态枚举
 */
UENUM(BlueprintType)
enum class EDBAAnimState : uint8
{
	Idle UMETA(DisplayName = "待机"),
	Walk UMETA(DisplayName = "行走"),
	Run UMETA(DisplayName = "奔跑"),
	Attack UMETA(DisplayName = "攻击"),
	Skill UMETA(DisplayName = "技能"),
	Hit UMETA(DisplayName = "受击"),
	Death UMETA(DisplayName = "死亡"),
};