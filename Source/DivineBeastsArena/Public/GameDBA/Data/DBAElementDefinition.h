// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DBAElementDefinition.generated.h"

/**
 * 自然元素之力定义行
 * 定义五种自然元素之力的克制关系、属性加成、显示信息
 * 自然元素之力是底层战斗键，影响伤害计算、技能组生成、共鸣效果
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAElementDefinitionRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 自然元素之力枚举值（对应 EDBAElement） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Element", meta = (DisplayName = "自然元素之力枚举"))
	uint8 ElementEnum = 0;

	/** 自然元素之力中文显示名称 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Element", meta = (DisplayName = "中文名称"))
	FText DisplayNameCN;

	/** 自然元素之力英文显示名称 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Element", meta = (DisplayName = "英文名称"))
	FText DisplayNameEN;

	/** 自然元素之力描述 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Element", meta = (DisplayName = "描述", MultiLine = true))
	FText Description;

	/** 自然元素之力图标路径 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Element", meta = (DisplayName = "图标路径"))
	TSoftObjectPtr<UTexture2D> IconTexture;

	/** 克制的自然元素之力枚举值 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Element|Counter", meta = (DisplayName = "克制元素"))
	uint8 CounterElementEnum = 0;

	/** 被克制的自然元素之力枚举值 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Element|Counter", meta = (DisplayName = "被克制元素"))
	uint8 CounteredByElementEnum = 0;

	/** 普通技能克制伤害倍率（默认 1.25） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Element|Counter", meta = (DisplayName = "普通技能克制倍率"))
	float NormalCounterMultiplier = 1.25f;

	/** 普通技能被克制伤害倍率（默认 0.80） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Element|Counter", meta = (DisplayName = "普通技能被克制倍率"))
	float NormalCounteredMultiplier = 0.80f;

	/** 生肖大招克制伤害倍率（默认 1.35） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Element|Counter", meta = (DisplayName = "生肖大招克制倍率"))
	float UltimateCounterMultiplier = 1.35f;

	/** 生肖大招被克制伤害倍率（默认 0.65） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Element|Counter", meta = (DisplayName = "生肖大招被克制倍率"))
	float UltimateCounteredMultiplier = 0.65f;

	/** 攻击力加成百分比（例如 Metal / Fire 为 15%） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Element|Bonus", meta = (DisplayName = "攻击力加成%"))
	float AttackPowerBonusPercent = 0.0f;

	/** 生命值加成百分比（例如 Wood / Water / Earth 为 15%） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Element|Bonus", meta = (DisplayName = "生命值加成%"))
	float MaxHealthBonusPercent = 0.0f;

	/** 防御力加成百分比（例如 Water / Earth 为 10%） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Element|Bonus", meta = (DisplayName = "防御力加成%"))
	float DefenseBonusPercent = 0.0f;

	/** 移动速度加成百分比（例如 Wood / Fire 为 10%） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Element|Bonus", meta = (DisplayName = "移速加成%"))
	float MoveSpeedBonusPercent = 0.0f;

	/** 暴击率加成百分比（例如 Metal 为 10%） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Element|Bonus", meta = (DisplayName = "暴击率加成%"))
	float CriticalRateBonusPercent = 0.0f;

	/** 护盾值加成百分比（例如 Metal 为 15%） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Element|Bonus", meta = (DisplayName = "护盾值加成%"))
	float ShieldBonusPercent = 0.0f;

	/** 生命回复加成百分比（例如 Wood 为 20%） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Element|Bonus", meta = (DisplayName = "生命回复加成%"))
	float HealthRegenBonusPercent = 0.0f;

	/** 能量回复加成百分比（例如 Water 为 20%） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Element|Bonus", meta = (DisplayName = "能量回复加成%"))
	float EnergyRegenBonusPercent = 0.0f;

	/** 伤害减少加成百分比（例如 Earth 为 5%） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Element|Bonus", meta = (DisplayName = "伤害减少加成%"))
	float DamageReductionBonusPercent = 0.0f;

	/** 自然元素之力主题色（用于技能特效、UI 高亮） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Element", meta = (DisplayName = "主题色"))
	FLinearColor ThemeColor = FLinearColor::White;

	/** 是否在当前版本可用 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Element", meta = (DisplayName = "是否可用"))
	bool bIsAvailable = true;

	FDBAElementDefinitionRow()
		: ElementEnum(0)
		, CounterElementEnum(0)
		, CounteredByElementEnum(0)
		, NormalCounterMultiplier(1.25f)
		, NormalCounteredMultiplier(0.80f)
		, UltimateCounterMultiplier(1.35f)
		, UltimateCounteredMultiplier(0.65f)
		, AttackPowerBonusPercent(0.0f)
		, MaxHealthBonusPercent(0.0f)
		, DefenseBonusPercent(0.0f)
		, MoveSpeedBonusPercent(0.0f)
		, CriticalRateBonusPercent(0.0f)
		, ShieldBonusPercent(0.0f)
		, HealthRegenBonusPercent(0.0f)
		, EnergyRegenBonusPercent(0.0f)
		, DamageReductionBonusPercent(0.0f)
		, ThemeColor(FLinearColor::White)
		, bIsAvailable(true)
	{
	}
};
