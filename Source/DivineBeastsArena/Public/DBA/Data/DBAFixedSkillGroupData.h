// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Common/Types/DBACommonTypes.h"
#include "DBAFixedSkillGroupData.generated.h"

/**
 * 生肖 × 自然元素之力固定技能组数据行
 *
 * 用途：
 * - 定义生肖 × 自然元素之力的固定技能组
 * - 提供技能组的所有技能 ID
 * - 提供技能组的元素共鸣等级
 * - 提供技能组的 AbilitySet 资产引用
 *
 * 重要：
 * - 12 生肖 × 5 种自然元素之力 = 60 条固定技能组
 * - 玩家不自由选择技能
 * - SkillLoadout 只展示系统生成结果与确认锁定
 * - 技能组由 Zodiac + Element 查表自动生成
 * - FiveCamp 只改变表现包，不改变固定技能组
 *
 * 固定技能组包含：
 * - Passive：被动技能，自动生效
 * - ActiveSkill01~04：主动技能，占用输入槽 Skill01~04
 * - ZodiacUltimate：生肖大招，占用输入槽 Ultimate
 *
 * 元素共鸣等级：
 * - 同元素技能数量决定共鸣等级
 * - 共鸣等级影响控制时间和护盾值
 *
 * 源表非法命名字段：
 * - 保留 OriginalSourceName 用于追溯
 * - 输出 SanitizedName 用于代码引用
 * - 输出 SanitizedAssetName 用于资产命名
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAZodiacElementFixedSkillGroupRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 行 ID（唯一标识符） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillGroup")
	FName RowId;

	/** 数据版本 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillGroup")
	int32 DataVersion = 1;

	/** 生肖类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillGroup")
	EDBAZodiac ZodiacType = EDBAZodiac::None;

	/** 自然元素之力类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillGroup")
	EDBAElement ElementType = EDBAElement::None;

	/** 被动技能 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillGroup|Skills")
	FName ElementPassiveSkillId;

	/** 元素技能 1 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillGroup|Skills")
	FName ElementSkill1Id;

	/** 元素技能 2 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillGroup|Skills")
	FName ElementSkill2Id;

	/** 元素技能 3 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillGroup|Skills")
	FName ElementSkill3Id;

	/** 元素技能 4 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillGroup|Skills")
	FName ElementSkill4Id;

	/** 生肖大招技能 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillGroup|Skills")
	FName ZodiacUltimateSkillId;

	/** 元素共鸣等级（0-4） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Resonance", meta = (ClampMin = "0", ClampMax = "4"))
	int32 ElementResonanceLevel = 0;

	/** 共鸣元素类型（通常与 ElementType 相同） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Resonance")
	EDBAElement ResonanceElement = EDBAElement::None;

	/** 共鸣控制时间加成（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Resonance")
	float ResonanceControlTimeBonus = 0.0f;

	/** 共鸣护盾值加成（百分比） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Resonance")
	float ResonanceShieldBonus = 0.0f;

	/** AbilitySet 资产引用 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillGroup|Resources")
	TSoftObjectPtr<UDataAsset> AbilitySetAsset;

	/** 技能组显示名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillGroup")
	FText DisplayName;

	/** 技能组描述 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillGroup")
	FText Description;

	/** 技能组图标 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillGroup")
	TSoftObjectPtr<UTexture2D> Icon;

	/** 原始源表名称（用于追溯非法命名） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillGroup|Naming")
	FString OriginalSourceName;

	/** 清理后的名称（用于代码引用） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillGroup|Naming")
	FString SanitizedName;

	/** 清理后的资产名称（用于资产命名） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillGroup|Naming")
	FString SanitizedAssetName;

	/** 是否启用 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillGroup")
	bool bEnabled = true;

	/** 是否在开发中 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillGroup")
	bool bIsInDevelopment = false;

	/** 策划备注 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillGroup")
	FText DesignerNotes;

	FDBAZodiacElementFixedSkillGroupRow()
	{
	}

	/**
	 * 获取技能组唯一键
	 * 格式：Zodiac_Element
	 */
	FString GetUniqueKey() const
	{
		return FString::Printf(TEXT("%s_%s"),
			*UEnum::GetValueAsString(ZodiacType),
			*UEnum::GetValueAsString(ElementType));
	}

	/**
	 * 计算元素共鸣等级
	 * 根据同元素技能数量计算
	 */
	int32 CalculateResonanceLevel() const
	{
		// 当前阶段简化实现，返回配置的共鸣等级
		// 未来可以根据技能表动态计算
		return ElementResonanceLevel;
	}
};

/**
 * 生肖英雄技能组汇总数据行
 *
 * 用途：
 * - 汇总单个生肖英雄的所有技能组
 * - 提供生肖英雄 × 5 种自然元素之力的技能组引用
 * - 用于快速查询和加载
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAZodiacHeroAbilitySetSummaryRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 生肖类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySetSummary")
	EDBAZodiac Zodiac = EDBAZodiac::None;

	/** Metal 元素技能组行 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySetSummary|SkillGroups")
	FName MetalSkillGroupRowId;

	/** Wood 元素技能组行 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySetSummary|SkillGroups")
	FName WoodSkillGroupRowId;

	/** Water 元素技能组行 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySetSummary|SkillGroups")
	FName WaterSkillGroupRowId;

	/** Fire 元素技能组行 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySetSummary|SkillGroups")
	FName FireSkillGroupRowId;

	/** Earth 元素技能组行 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySetSummary|SkillGroups")
	FName EarthSkillGroupRowId;

	/** 是否所有技能组都已配置 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySetSummary")
	bool bAllSkillGroupsConfigured = false;

	/** 配置完成度（0.0 - 1.0） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySetSummary", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ConfigurationCompleteness = 0.0f;

	/** 最后更新时间（Unix 时间戳） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySetSummary")
	int64 LastUpdateTime = 0;

	/** 策划负责人 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySetSummary")
	FString DesignerOwner;

	FDBAZodiacHeroAbilitySetSummaryRow()
	{
	}

	/**
	 * 获取指定元素的技能组行 ID
	 */
	FName GetSkillGroupRowId(EDBAElement Element) const
	{
		switch (Element)
		{
		case EDBAElement::Gold:
			return MetalSkillGroupRowId;
		case EDBAElement::Wood:
			return WoodSkillGroupRowId;
		case EDBAElement::Water:
			return WaterSkillGroupRowId;
		case EDBAElement::Fire:
			return FireSkillGroupRowId;
		case EDBAElement::Earth:
			return EarthSkillGroupRowId;
		default:
			return NAME_None;
		}
	}
};
