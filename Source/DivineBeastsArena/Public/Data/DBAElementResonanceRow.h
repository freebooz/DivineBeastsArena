// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Common/Types/DBACommonTypes.h"
#include "DBAElementResonanceRow.generated.h"

/**
 * 元素共鸣数据行
 *
 * 用途：
 * - 定义 5 种自然元素之力 × 5 个共鸣等级的属性加成
 * - 同元素技能数量决定共鸣等级
 * - 共鸣等级影响控制时间和护盾值
 *
 * 数据行数：25（5 元素 × 5 共鸣等级）
 *
 * 共鸣等级定义：
 * - Level 0：无共鸣（0 个同元素技能）
 * - Level 1：初级共鸣（1 个同元素技能）
 * - Level 2：中级共鸣（2 个同元素技能）
 * - Level 3：高级共鸣（3 个同元素技能）
 * - Level 4：超级共鸣（4 个同元素技能 + 1 个异元素技能）
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAElementResonanceRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 元素类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Resonance")
	EDBAElement ElementType = EDBAElement::None;

	/** 共鸣等级（0-4） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Resonance", meta = (ClampMin = "0", ClampMax = "4"))
	int32 ResonanceLevel = 0;

	/** 共鸣名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Resonance")
	FText DisplayName;

	/** 共鸣描述 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Resonance")
	FText Description;

	/** 共鸣图标 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Resonance")
	TSoftObjectPtr<UTexture2D> Icon;

	/** 伤害加成（百分比） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Resonance|Damage")
	float DamageBonus = 0.0f;

	/** 防御加成（百分比） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Resonance|Defense")
	float DefenseBonus = 0.0f;

	/** 生命值加成（百分比） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Resonance|Health")
	float HealthBonus = 0.0f;

	/** 能量恢复加成（百分比） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Resonance|Energy")
	float EnergyRegenBonus = 0.0f;

	/** 移动速度加成（百分比） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Resonance|MoveSpeed")
	float MoveSpeedBonus = 0.0f;

	/** 控制时间加成（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Resonance|Control")
	float ControlTimeBonus = 0.0f;

	/** 护盾值加成（百分比） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Resonance|Shield")
	float ShieldBonus = 0.0f;

	/** 共鸣特效名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Resonance|VFX")
	FName ResonanceVFXName;

	/** 共鸣音效名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Element|Resonance|SFX")
	FName ResonanceSFXName;

	FDBAElementResonanceRow()
	{
	}
};
