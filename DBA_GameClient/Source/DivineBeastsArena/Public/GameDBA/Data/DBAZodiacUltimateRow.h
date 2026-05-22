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
#include "GameCore/Types/DBACommonTypes.h"
#include "DBAZodiacUltimateRow.generated.h"

/**
 * 生肖终极技能数据行
 *
 * 用途：
 * - 定义 12 种生肖的终极技能
 * - 生肖大招是每个英雄的核心技能
 * - 每个生肖只有一个终极技能
 *
 * 数据行数：12（12 生肖）
 *
 * 终极技能特点：
 * - 占用 Ultimate 输入槽
 * - 消耗 UltimateEnergy
 * - 由生肖 × 自然元素之力共同决定效果
 * - 每个生肖大招有独特的视觉效果和特效
 *
 * 注意：
 * - ZodiacUltimate 是生肖大招语义
 * - 每个生肖必须有一个 ZodiacUltimate 语义
 * - ZodiacUltimate 输入槽为 Ultimate
 * - ZodiacUltimate 消耗 UltimateEnergy
 * - ZodiacUltimate 的自然元素之力属性由当前 PrimaryElement 决定
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAZodiacUltimateRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 生肖类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Ultimate")
	EDBAZodiac ZodiacType = EDBAZodiac::None;

	/** 生肖大招技能 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Ultimate")
	FName ZodiacUltimateSkillId;

	/** 生肖大招名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Ultimate")
	FText DisplayName;

	/** 生肖大招描述 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Ultimate")
	FText Description;

	/** 生肖大招图标 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Ultimate")
	TSoftObjectPtr<UTexture2D> Icon;

	/** 生肖大招缩放图标 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Ultimate")
	TSoftObjectPtr<UTexture2D> IconSilhouette;

	/** 冷却时间（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Ultimate|Config")
	float Cooldown = 120.0f;

	/** 终极能量消耗 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Ultimate|Config")
	float UltimateEnergyCost = 100.0f;

	/** 施法时间（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Ultimate|Config")
	float CastTime = 1.5f;

	/** 施法距离 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Ultimate|Config")
	float CastRange = 500.0f;

	/** 效果范围 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Ultimate|Config")
	float EffectRadius = 200.0f;

	/** 基础伤害值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Ultimate|Damage")
	float BaseDamage = 800.0f;

	/** 伤害缩放系数 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Ultimate|Damage")
	float DamageScaling = 2.5f;

	/** 控制时间（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Ultimate|Control")
	float ControlTime = 1.0f;

	/** 视觉效果类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Ultimate|VFX")
	FName VFXType;

	/** 视觉效果资源 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Ultimate|VFX")
	TSoftObjectPtr<UParticleSystem> VFXAsset;

	/** 音效类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Ultimate|SFX")
	FName SFXType;

	/** 音效资源 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Ultimate|SFX")
	TSoftObjectPtr<USoundBase> SFXAsset;

	/** 动画通知 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Ultimate|Animation")
	FName AnimNotifyName;

	/** 技能标签 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Ultimate")
	TArray<FName> SkillTags;

	/** 是否启用 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Ultimate")
	bool bEnabled = true;

	/** 是否在开发中 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Ultimate")
	bool bIsInDevelopment = false;

	FDBAZodiacUltimateRow()
	{
	}
};
