// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：声明技能名称 DataTable 行结构，承载 12 生肖 × 6 技能槽位的中文名称数据。
- 阅读重点：先看 USTRUCT 字段，再看 BuildSkillNameRowName 行名构造工具。
- 修改提示：新增技能名称字段时同步更新 CSV 源文件与 Subsystem 查询接口。
*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameDBA/Core/DBAEnumsCore.h"
#include "DBASkillNameRow.generated.h"

/**
 * 技能名称 DataTable 行结构
 *
 * 用途：
 * - 将 DBAConstants::DBASkillNames 中硬编码的 72 条技能名称迁移至 DataTable 驱动
 * - 支持本地化（FText）和设计师调参
 *
 * 行名规则：BuildSkillNameRowName(Zodiac, SkillSlotIndex)
 * 例如：Rat_Passive、Rat_Skill01、Rat_Ultimate
 *
 * SkillSlotIndex 约定：
 * - 0 = Passive（被动技能）
 * - 1 = Skill01
 * - 2 = Skill02
 * - 3 = Skill03
 * - 4 = Skill04
 * - 5 = Ultimate（生肖大招）
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBASkillNameRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 生肖类型 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillName")
	EDBAZodiacType Zodiac = EDBAZodiacType::None;

	/** 技能槽位索引（0=Passive, 1~4=Skill01~04, 5=Ultimate） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillName", meta = (ClampMin = "0", ClampMax = "5"))
	int32 SkillSlotIndex = 0;

	/** 技能槽位名称（Passive / Skill01 / Skill02 / Skill03 / Skill04 / Ultimate） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillName")
	FString SkillSlotName;

	/** 技能中文名称（本地化文本，UI 显示用） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillName")
	FText DisplayName;

	/** 技能英文名称（可选，便于跨团队沟通） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillName")
	FString EnglishName;
};

/**
 * 构建技能名称 DataTable 行名
 *
 * 格式：{Zodiac}_{SlotName}，例如 Rat_Passive、Tiger_Skill03、Pig_Ultimate
 *
 * @param Zodiac 生肖类型
 * @param SkillSlotIndex 技能槽位索引（0=Passive, 1~4=Skill01~04, 5=Ultimate）
 * @return 行名字符串
 */
DIVINEBEASTSARENA_API FName BuildSkillNameRowName(EDBAZodiacType Zodiac, int32 SkillSlotIndex);

/** 根据技能槽位索引返回槽位名称字符串（Passive / Skill01 / ... / Ultimate） */
DIVINEBEASTSARENA_API const TCHAR* GetSkillSlotNameByIndex(int32 SkillSlotIndex);
