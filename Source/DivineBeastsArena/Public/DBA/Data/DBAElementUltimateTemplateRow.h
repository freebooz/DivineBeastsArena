// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Common/Types/DBACommonTypes.h"
#include "DBAElementUltimateTemplateRow.generated.h"

/**
 * 自然元素之力终极技能模板数据行
 *
 * 用途：
 * - 定义 5 种自然元素之力的终极技能模板
 * - 生肖大招调用此模板生成具体技能
 * - 模板定义终极技能的基础效果
 *
 * 数据行数：5（Metal/Wood/Water/Fire/Earth）
 *
 * 终极技能特点：
 * - 占用 Ultimate 输入槽
 * - 消耗 UltimateEnergy
 * - 冷却时间较长
 * - 效果强大
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAElementUltimateTemplateRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 元素类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Ultimate")
	EDBAElement ElementType = EDBAElement::None;

	/** 终极技能模板 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Ultimate")
	FName UltimateTemplateId;

	/** 终极技能名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Ultimate")
	FText DisplayName;

	/** 终极技能描述 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Ultimate")
	FText Description;

	/** 终极技能图标 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Ultimate")
	TSoftObjectPtr<UTexture2D> Icon;

	/** 冷却时间（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Ultimate|Config")
	float Cooldown = 120.0f;

	/** 终极能量消耗 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Ultimate|Config")
	float UltimateEnergyCost = 100.0f;

	/** 施法时间（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Ultimate|Config")
	float CastTime = 1.0f;

	/** 施法距离 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Ultimate|Config")
	float CastRange = 1000.0f;

	/** 效果范围 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Ultimate|Config")
	float EffectRadius = 300.0f;

	/** 基础伤害值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Ultimate|Damage")
	float BaseDamage = 500.0f;

	/** 伤害缩放系数 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Ultimate|Damage")
	float DamageScaling = 2.0f;

	/** 元素共鸣伤害加成（百分比） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Ultimate|Resonance")
	float ResonanceDamageBonus = 0.0f;

	/** 控制时间（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Ultimate|Control")
	float ControlTime = 0.0f;

	/** 护盾值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Ultimate|Shield")
	float ShieldValue = 0.0f;

	/** 护盾持续时间（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Ultimate|Shield")
	float ShieldDuration = 0.0f;

	/** 视觉效果类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Ultimate|VFX")
	FName VFXType;

	/** 音效类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Ultimate|SFX")
	FName SFXType;

	/** 技能标签 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Ultimate")
	TArray<FName> SkillTags;

	/** 是否启用 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Ultimate")
	bool bEnabled = true;

	/** 是否在开发中 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Ultimate")
	bool bIsInDevelopment = false;

	FDBAElementUltimateTemplateRow()
	{
	}
};
