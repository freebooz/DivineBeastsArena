// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Types/DBACommonEnums.h"
#include "GameDBA/Core/DBAEnumsCore.h"
#include "DBADamageCalculator.generated.h"

/**
 * DBADamageCalculator
 *
 * 伤害计算器
 * 计算技能伤害，考虑元素克制、共鸣加成、连锁加成
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBADamageCalculator : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * 计算最终伤害
	 *
	 * @param BaseDamage 基础伤害
	 * @param AttackElement 攻击方元素
	 * @param DefenseElement 防御方元素
	 * @param ResonanceLevel 共鸣等级 (0-4)
	 * @param ChainLevel 连锁等级 (0-10)
	 * @return 最终伤害值
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Damage")
	static float CalculateDamage(
		float BaseDamage,
		EDBAElement AttackElement,
		EDBAElement DefenseElement,
		int32 ResonanceLevel,
		int32 ChainLevel);

	/**
	 * 获取元素克制倍率
	 *
	 * @param AttackElement 攻击方元素
	 * @param DefenseElement 防御方元素
	 * @return 克制倍率 (1.2/1.0/0.8)
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Damage")
	static float GetElementMultiplier(EDBAElement AttackElement, EDBAElement DefenseElement);

	/**
	 * 获取共鸣伤害加成
	 *
	 * @param ResonanceLevel 共鸣等级 (0-4)
	 * @return 加成百分比 (0.0 ~ 0.2)
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Damage")
	static float GetResonanceBonus(int32 ResonanceLevel);

	/**
	 * 获取连锁伤害倍率
	 *
	 * @param ChainLevel 连锁等级 (0-10)
	 * @return 连锁倍率 (1.0 ~ 1.35 或 0.2最大生命)
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Damage")
	static float GetChainMultiplier(int32 ChainLevel);

	/**
	 * 获取连锁是否终结
	 *
	 * @param ChainLevel 连锁等级
	 * @return true if chain is at max level (10)
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Damage")
	static bool IsChainFinal(int32 ChainLevel);

	/**
	 * 计算最终伤害（包含防御减免和暴击）
	 *
	 * @param BaseDamage 基础伤害
	 * @param AttackElement 攻击方元素
	 * @param DefenseElement 防御方元素
	 * @param ResonanceLevel 共鸣等级 (0-4)
	 * @param ChainLevel 连锁等级 (0-10)
	 * @param Defense 防御力
	 * @param CriticalRate 暴击率 (0-1)
	 * @param CriticalMultiplier 暴击倍率
	 * @param OutbIsCritical 输出参数，是否暴击
	 * @return 最终伤害值
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Damage")
	static float CalculateFinalDamage(
		float BaseDamage,
		EDBAElement AttackElement,
		EDBAElement DefenseElement,
		int32 ResonanceLevel,
		int32 ChainLevel,
		float Defense,
		float CriticalRate,
		float CriticalMultiplier,
		bool& OutbIsCritical);

	/**
	 * 应用伤害到目标
	 *
	 * @param Attacker 攻击者
	 * @param Target 目标
	 * @param FinalDamage 最终伤害
	 * @param Element 元素类型
	 * @param bIsCritical 是否暴击
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Damage")
	static void ApplyDamageToTarget(
		AActor* Attacker,
		AActor* Target,
		float FinalDamage,
		EDBAElement Element,
		bool bIsCritical);
};
