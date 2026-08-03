// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：声明万象灵庭 V2 生肖战术技能定义 DataTable 行结构（12×5=60 行）。
- 阅读重点：FDBAZodiacSkillDefinitionRow 对应 DT_ZodiacSkillDefinitions.csv 源表列。
- 修改提示：新增字段时同步更新 CSV 源文件、gen_zodiac_skill_definitions_csv.py 与架构文档第六节。
*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameDBA/Core/DBAEnumsCore.h"
#include "DBAZodiacSkillDefinitionRow.generated.h"

/**
 * 万象灵庭 V2 生肖战术技能定义行
 *
 * 用途：
 * - 承载每个生肖 5 个战术技能（攻击/移动/控制/功能/防护保命）的策划数据
 * - 与 V15 的 DT_SkillNames（6 槽位：被动+4主动+大招）并行存在，供 GAS/UI 逐步迁移
 *
 * 行名规则：{CharacterCodename}_{SkillType}
 * 例如：Shadowfang_Attack、Blackhorn_Defense
 *
 * CSV 源：Content/DBA/Data/Tables/Source/DT_ZodiacSkillDefinitions.csv
 * 设计文档：docs/Architecture/Characters/ZodiacSkillDesign_V2_万象灵庭.md
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAZodiacSkillDefinitionRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 技能唯一 ID（如 SKL_Shadowfang_BiteShadow） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ZodiacSkill")
	FName SkillID;

	/** 角色资产 ID（如 CHRShadowfang） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ZodiacSkill")
	FName CharacterID;

	/** 生肖枚举（Rat/Ox/...） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ZodiacSkill")
	EDBAZodiacType Zodiac = EDBAZodiacType::None;

	/** 角色全称（如 子鼠·影牙灵君） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ZodiacSkill")
	FString ZodiacDisplayName;

	/** 战术技能类型（攻击/移动/控制/功能/防护） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ZodiacSkill")
	EDBAZodiacTacticalSkillType SkillType = EDBAZodiacTacticalSkillType::None;

	/** 技能中文名（如 咬影） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ZodiacSkill")
	FText DisplayName;

	/** 技能英文代号（如 BiteShadow） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ZodiacSkill")
	FName EnglishSkillName;

	/** 基础冷却（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ZodiacSkill|Timing", meta = (ClampMin = "0"))
	float BaseCD = 0.f;

	/** 施法时间（秒，0=待填） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ZodiacSkill|Timing", meta = (ClampMin = "0"))
	float CastTime = 0.f;

	/** 施法/突进距离（0=待填） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ZodiacSkill|Range", meta = (ClampMin = "0"))
	float Range = 0.f;

	/** 效果半径（0=待填） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ZodiacSkill|Range", meta = (ClampMin = "0"))
	float Radius = 0.f;

	/** 持续时长（秒，0=待填） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ZodiacSkill|Timing", meta = (ClampMin = "0"))
	float Duration = 0.f;

	/** 伤害系数（0=待策划填数） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ZodiacSkill|Numeric", meta = (ClampMin = "0"))
	float DamageCoefficient = 0.f;

	/** 护盾系数（0=待填） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ZodiacSkill|Numeric", meta = (ClampMin = "0"))
	float ShieldCoefficient = 0.f;

	/** 治疗系数（0=待填） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ZodiacSkill|Numeric", meta = (ClampMin = "0"))
	float HealCoefficient = 0.f;

	/** 控制时长（秒，0=待填） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ZodiacSkill|Numeric", meta = (ClampMin = "0"))
	float ControlDuration = 0.f;

	/** 最大叠层（0=不适用） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ZodiacSkill|Numeric", meta = (ClampMin = "0"))
	int32 StackMax = 0;

	/** 条件标签（分号分隔，如 背袭;残血;嗅弱） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ZodiacSkill|Design")
	FString ConditionTag;

	/** 反制标签（分号分隔，如 可揭露;可净化） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ZodiacSkill|Design")
	FString CounterTag;

	/** VFX 资产命名（策划/美术对齐，如 VFXShadowfang_BiteShadow） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ZodiacSkill|Assets")
	FName VFXAssetName;

	/** 图标资产命名（如 ICOShadowfang_BiteShadow） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ZodiacSkill|Assets")
	FName IconAssetName;

	/** 语音行 ID（如 VO_Shadowfang_BiteShadow） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ZodiacSkill|Assets")
	FName VoiceLineID;

	/** 技能释放台词（中文，供 UI/语音脚本对齐） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ZodiacSkill|Assets")
	FString VoiceLineText;

	/** 数值接口说明（策划填表指引） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ZodiacSkill|Design")
	FString NumericInterface;

	/** 平衡备注 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ZodiacSkill|Design")
	FString BalanceNote;

	/** 是否启用 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ZodiacSkill")
	bool bEnabled = true;
};

/** 构建 V2 技能定义行名：{CharacterCodename}_{SkillType} */
DIVINEBEASTSARENA_API FName BuildZodiacSkillDefinitionRowName(FName CharacterCodename, EDBAZodiacTacticalSkillType SkillType);

/** 将战术技能类型枚举转为 CSV/行名字符串（Attack/Move/...） */
DIVINEBEASTSARENA_API const TCHAR* GetZodiacTacticalSkillTypeName(EDBAZodiacTacticalSkillType SkillType);
