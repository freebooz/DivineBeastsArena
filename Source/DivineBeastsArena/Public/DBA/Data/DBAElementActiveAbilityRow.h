// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Common/Types/DBACommonTypes.h"
#include "DBAElementActiveAbilityRow.generated.h"

/**
 * 自然元素之力主动技能数据行
 *
 * 用途：
 * - 定义 5 种自然元素之力的主动技能（每元素 4 个）
 * - 主动技能占用 Skill01-Skill04 输入槽
 * - 主动技能有冷却时间、能量消耗
 *
 * 数据行数：25（5 元素 × 5 技能位置，其中位置 0 为占位）
 *
 * 技能位置：
 * - Position 0：占位符（无技能）
 * - Position 1-4：主动技能 01-04
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAElementActiveAbilityRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 元素类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Active")
	EDBAElement ElementType = EDBAElement::None;

	/** 技能位置（0=占位，1-4=主动技能） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Active", meta = (ClampMin = "0", ClampMax = "4"))
	int32 SkillPosition = 0;

	/** 主动技能 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Active")
	FName ActiveSkillId;

	/** 主动技能名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Active")
	FText DisplayName;

	/** 主动技能描述 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Active")
	FText Description;

	/** 主动技能图标 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Active")
	TSoftObjectPtr<UTexture2D> Icon;

	/** 技能类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Active")
	EDBASkillType SkillType = EDBASkillType::Active;

	/** 冷却时间（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Active|Config")
	float Cooldown = 5.0f;

	/** 能量消耗 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Active|Config")
	float EnergyCost = 20.0f;

	/** 施法时间（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Active|Config")
	float CastTime = 0.5f;

	/** 施法距离 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Active|Config")
	float CastRange = 500.0f;

	/** 效果范围 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Active|Config")
	float EffectRadius = 0.0f;

	/** 基础伤害值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Active|Damage")
	float BaseDamage = 100.0f;

	/** 伤害缩放系数 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Active|Damage")
	float DamageScaling = 1.0f;

	/** 元素共鸣加成（百分比） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Active|Resonance")
	float ResonanceDamageBonus = 0.0f;

	/** 控制时间加成（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Active|Control")
	float ControlTimeBonus = 0.0f;

	/** 视觉效果类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Active|VFX")
	FName VFXType;

	/** 音效类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Active|SFX")
	FName SFXType;

	/** 技能标签 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Active")
	TArray<FName> SkillTags;

	/** 是否启用 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Active")
	bool bEnabled = true;

	/** 是否在开发中 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Active")
	bool bIsInDevelopment = false;

	FDBAElementActiveAbilityRow()
	{
	}
};
