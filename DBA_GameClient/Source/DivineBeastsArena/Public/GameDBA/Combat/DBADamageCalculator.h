// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameCore/Types/DBACommonEnums.h"
#include "GameDBA/Core/DBAEnumsCore.h"
#include "GameDBA/Combat/Structs/DBADamageTypes.h"
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

	/**
	 * 应用伤害并触发 GameplayCue。
	 *
	 * 兼容两类目标：
	 * - 带 AbilitySystemComponent 的战斗单位：直接更新 BattleAttributeSet。
	 * - 传统 Actor / 怪物：回退到 TakeDamage，保留已有怪物受击逻辑。
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Damage")
	static void ApplyDamageToTargetWithCue(
		AActor* Attacker,
		AActor* Target,
		float FinalDamage,
		EDBAElement Element,
		bool bIsCritical,
		const FGameplayTag& GameplayCueTag,
		FVector HitLocation);

	// ========== Value Object 方法 (使用 EDBAElementType) ==========

	/**
	 * 使用 Value Object 计算最终伤害 (EDBAElementType 版本)
	 *
	 * @param Params 伤害计算参数
	 * @return 最终伤害结果，包含所有计算细节
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Damage")
	static FFinalDamageResult CalculateFinalDamageWithObject(const FDamageCalculationParams& Params);

	/**
	 * 获取元素克制倍率 (EDBAElementType 版本)
	 *
	 * @param AttackElement 攻击方元素
	 * @param DefenseElement 防御方元素
	 * @return 元素克制结果
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Damage")
	static FElementCounterResult GetElementCounterResult(EDBAElementType AttackElement, EDBAElementType DefenseElement);

	/**
	 * 获取共鸣伤害加成 (EDBAElementType 版本)
	 *
	 * @param ResonanceLevel 共鸣等级 (0-4)
	 * @return 共鸣加成百分比 (0.0 ~ 0.2)
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Damage")
	static float GetResonanceBonusForElement(EDBAResonanceLevel ResonanceLevel);

	/**
	 * 获取连锁加成 (EDBAElementType 版本)
	 *
	 * @param ChainLevel 连锁等级 (0-10)
	 * @return 连锁加成
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Damage")
	static FChainBonus GetChainBonus(int32 ChainLevel);

	/**
	 * 从旧枚举 EDBAElement 转换为 EDBAElementType
	 * 用于兼容 GameCore 模块的旧代码
	 */
	static EDBAElementType GetElementTypeFromOldEnum(EDBAElement OldElement);
};
