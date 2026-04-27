// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Common/Types/DBACommonTypes.h"
#include "DBAResonanceBonusData.generated.h"

/**
 * 共鸣加成 GameplayEffect 配置数据行
 *
 * 用途：
 * - 定义共鸣等级对应的 GameplayEffect 配置
 * - 用于通过数据表驱动共鸣效果
 * - 支持通过 UDataTable::FindRow 查表获取
 *
 * 数据行数：25（5 元素 × 5 共鸣等级）
 *
 * 共鸣等级定义：
 * - Level 0：无共鸣（0 个同元素技能）
 * - Level 1：初级共鸣（2 个同元素技能）
 * - Level 2：中级共鸣（3 个同元素技能）
 * - Level 3：高级共鸣（4 个同元素技能）
 * - Level 4：超级共鸣（5 个同元素技能）
 *
 * 使用方式：
 * - 在 UE 编辑器中创建 DataTable，选择此类作为行类型
 * - 表名建议：DBA_ResonanceBonusDataTable
 * - 行命名格式：{Element}_{Level}，如 Metal_1, Wood_2
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAResonanceBonusDataRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 共鸣等级（0-4） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance|Bonus", meta = (ClampMin = "0", ClampMax = "4"))
	int32 ResonanceLevel = 0;

	/** 伤害加成（百分比） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance|Bonus")
	float DamageBonusPercent = 0.0f;

	/** 防御加成（百分比） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance|Bonus")
	float DefenseBonusPercent = 0.0f;

	/** 生命值加成（百分比） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance|Bonus")
	float HealthBonusPercent = 0.0f;

	/** 能量恢复加成（百分比） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance|Bonus")
	float EnergyRegenBonusPercent = 0.0f;

	/** 移动速度加成（百分比） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance|Bonus")
	float MoveSpeedBonusPercent = 0.0f;

	/** 控制时间加成（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance|Bonus")
	float ControlTimeBonusSeconds = 0.0f;

	/** 护盾值加成（百分比） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance|Bonus")
	float ShieldBonusPercent = 0.0f;

	/** GameplayEffect 资产路径（可选） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance|Bonus")
	TSoftObjectPtr<UDataAsset> GameplayEffectAsset;

	FDBAResonanceBonusDataRow()
	{
	}

	/**
	 * 获取行名称
	 * 格式：{Element}_{Level}
	 */
	FString GetRowName(EDBAElement Element) const
	{
		return FString::Printf(TEXT("%s_%d"),
			*UEnum::GetValueAsString(Element),
			ResonanceLevel);
	}

	/**
	 * 根据元素和等级查找数据行
	 * @param DataTable 数据表
	 * @param Element 元素类型
	 * @param Level 共鸣等级
	 * @return 数据行指针
	 */
	static FDBAResonanceBonusDataRow* FindRow(UDataTable* DataTable, EDBAElement Element, int32 Level)
	{
		if (!DataTable)
		{
			return nullptr;
		}

		FString RowName = FString::Printf(TEXT("%s_%d"),
			*UEnum::GetValueAsString(Element),
			Level);

		return DataTable->FindRow<FDBAResonanceBonusDataRow>(FName(RowName), TEXT(""));
	}
};
