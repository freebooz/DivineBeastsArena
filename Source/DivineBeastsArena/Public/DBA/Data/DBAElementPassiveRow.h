// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Common/Types/DBACommonTypes.h"
#include "DBAElementPassiveRow.generated.h"

/**
 * 自然元素之力被动技能数据行
 *
 * 用途：
 * - 定义 5 种自然元素之力的被动技能
 * - 被动技能自动生效，无需玩家操作
 * - 被动技能提供元素共鸣加成
 *
 * 数据行数：5（Metal/Wood/Water/Fire/Earth）
 *
 * 被动技能类型：
 * - ResonancePassive：元素共鸣被动，永久生效
 * - ConditionalPassive：条件触发被动，满足条件时生效
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAElementPassiveRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 元素类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Passive")
	EDBAElement ElementType = EDBAElement::None;

	/** 被动技能 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Passive")
	FName PassiveSkillId;

	/** 被动技能名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Passive")
	FText DisplayName;

	/** 被动技能描述 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Passive")
	FText Description;

	/** 被动技能图标 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Passive")
	TSoftObjectPtr<UTexture2D> Icon;

	/** 被动技能类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Passive")
	EDBAPassiveType PassiveType = EDBAPassiveType::ElementResonance;

	/** 元素共鸣等级要求（0-4） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Passive", meta = (ClampMin = "0", ClampMax = "4"))
	int32 RequiredResonanceLevel = 0;

	/** 共鸣控制时间加成（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Passive|ResonanceBonus")
	float ResonanceControlTimeBonus = 0.0f;

	/** 共鸣护盾值加成（百分比） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Passive|ResonanceBonus")
	float ResonanceShieldBonus = 0.0f;

	/** 共鸣伤害加成（百分比） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Passive|ResonanceBonus")
	float ResonanceDamageBonus = 0.0f;

	/** 共鸣能量恢复加成（百分比） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Passive|ResonanceBonus")
	float ResonanceEnergyRegenBonus = 0.0f;

	/** 被动技能效果覆盖标签 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Passive")
	TArray<FName> EffectTags;

	/** 是否启用 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Passive")
	bool bEnabled = true;

	/** 是否在开发中 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Passive")
	bool bIsInDevelopment = false;

	FDBAElementPassiveRow()
	{
	}
};
