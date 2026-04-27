// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "DBEEnergyCostEffect.generated.h"

/**
 * 能量消耗 GameplayEffect
 *
 * 用途：
 * - 用于技能消耗 CurrentEnergy
 * - 通过 GAS 预测和复制系统确保客户端/服务端同步
 * - 替代直接修改属性的 TECHDEBT 方案
 *
 * 使用方式：
 * - 在 Ability 的 CostGameplayEffect 中配置此类
 * - 或通过 UDBAMobaAbilitySystemComponentBase::GrantEffect 应用
 */
UCLASS(BlueprintType, meta = (DisplayName = "DBA Energy Cost Effect"))
class DIVINEBEASTSARENA_API UDBEEnergyCostEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UDBEEnergyCostEffect();
};
