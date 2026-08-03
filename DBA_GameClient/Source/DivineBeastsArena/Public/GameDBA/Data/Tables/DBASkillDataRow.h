// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Sound/SoundCue.h"
#include "GameDBA/Core/DBAEnumsCore.h"
#include "GameCore/Types/DBACommonTypes.h"
#include "DBASkillDataRow.generated.h"

/**
 * 技能数据行（统一技能表）
 *
 * 用途：
 * - 定义所有技能的统一数据
 * - 包括被动技能、主动技能、终极技能
 * - 支持技能升级和技能点配置
 *
 * 技能分类：
 * - Passive：被动技能
 * - Active01-04：主动技能（对应 Skill01-04 槽位）
 * - Ultimate：终极技能
 *
 * 技能数据结构：
 * - 包含技能属性、效果、冷却、消耗
 * - 包含技能升级数据
 * - 包含技能动画和特效资源
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBASkillDataRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 技能 ID（唯一标识符） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	FName SkillId;

	/** 技能名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	FText DisplayName;

	/** 技能描述 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	FText Description;

	/** 技能图标 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	TSoftObjectPtr<UTexture2D> Icon;

	/** 技能类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	EDBASkillType SkillType = EDBASkillType::Active;

	/** 技能语义 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	EDBASkillSlot SkillSlot = EDBASkillSlot::None;

	/** 所属元素 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	EDBAElement ElementType = EDBAElement::None;

	/** 所属生肖 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	EDBAZodiac ZodiacType = EDBAZodiac::None;

	/** 冷却时间（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Config")
	float Cooldown = 5.0f;

	/** 能量消耗 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Config")
	float EnergyCost = 0.0f;

	/** 终极能量消耗（仅 Ultimate） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Config")
	float UltimateEnergyCost = 0.0f;

	/** 施法时间（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Config")
	float CastTime = 0.0f;

	/** 施法距离 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Config")
	float CastRange = 0.0f;

	/** 效果范围 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Config")
	float EffectRadius = 0.0f;

	/** 基础伤害 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Damage")
	float BaseDamage = 0.0f;

	/** 伤害缩放系数 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Damage")
	float DamageScaling = 0.0f;

	/** 治疗量 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Heal")
	float HealAmount = 0.0f;

	/** 治疗缩放系数 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Heal")
	float HealScaling = 0.0f;

	/** 护盾值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Shield")
	float ShieldValue = 0.0f;

	/** 控制时间（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Control")
	float ControlTime = 0.0f;

	/** 视觉效果类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|VFX")
	FName VFXType;

	/** 视觉效果资源 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|VFX")
	TSoftObjectPtr<UParticleSystem> VFXAsset;

	/** 音效类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|SFX")
	FName SFXType;

	/** 音效资源 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|SFX")
	TSoftObjectPtr<USoundBase> SFXAsset;

	/** 动画蓝图类 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Animation")
	FSoftClassPath AnimBlueprintClass;

	/** 技能标签 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	TArray<FName> SkillTags;

	/** 技能战术定位（攻击/位移/控制/防护/功能） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Design")
	FName TacticalRole;

	/** 是否保命/防护技能（用于 UI 强调与新手引导） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Design")
	bool bIsDefensiveSkill = false;

	/** 图标命名（策划/美术对齐用） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Design")
	FText IconDesignName;

	/** 图标设计说明 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Design")
	FText IconDesignDescription;

	/** 特效表现说明 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Design")
	FText VFXDesignDescription;

	/** 技能升级配置（JSON 格式） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Upgrade")
	FString UpgradeConfigJson;

	/** 最大技能等级 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Upgrade", meta = (ClampMin = "1", ClampMax = "5"))
	int32 MaxSkillLevel = 1;

	/** 每级升级所需熟练度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Upgrade")
	int32 UpgradeProficiencyRequired = 1000;

	/** 是否启用 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	bool bEnabled = true;

	/** 是否在开发中 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	bool bIsInDevelopment = false;

	FDBASkillDataRow()
	{
	}
};
