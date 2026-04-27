// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "DBEResonanceBuffEffect.generated.h"

/**
 * 共鸣增益 GameplayEffect
 *
 * 用途：
 * - 用于应用元素共鸣的属性加成
 * - 根据共鸣等级应用不同的属性修饰符
 * - 被动生效，持续到技能组切换
 *
 * 属性加成（基于共鸣等级）：
 * - Lv.1: 伤害+5%, 防御+2%, 生命+3%, 控制时间+0.25秒, 护盾+5%
 * - Lv.2: 伤害+10%, 防御+4%, 生命+6%, 控制时间+0.50秒, 护盾+10%
 * - Lv.3: 伤害+15%, 防御+6%, 生命+9%, 控制时间+0.75秒, 护盾+15%
 * - Lv.4: 伤害+20%, 防御+8%, 生命+12%, 控制时间+1.00秒, 护盾+20%
 *
 * 使用方式：
 * - 通过 UDBAResonanceAbilityBase::ApplyResonanceEffect 应用
 * - 根据共鸣等级动态配置修饰符数值
 */
UCLASS(BlueprintType, meta = (DisplayName = "DBA Resonance Buff Effect"))
class DIVINEBEASTSARENA_API UDBEResonanceBuffEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UDBEResonanceBuffEffect();

	/** 当前共鸣等级（1-4） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Resonance")
	int32 ResonanceLevel = 1;

public:
	/** 根据共鸣等级配置修饰符 */
	void ConfigureForResonanceLevel(int32 Level);
};
