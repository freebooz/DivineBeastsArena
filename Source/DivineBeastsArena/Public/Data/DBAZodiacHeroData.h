// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Sound/SoundCue.h"
#include "Common/Types/DBACommonTypes.h"
#include "DBAZodiacHeroData.generated.h"

/**
 * 生肖英雄显示数据行
 *
 * 用途：
 * - 定义 12 生肖英雄的显示信息
 * - 提供生肖英雄中文名称、英文名称、描述
 * - 提供生肖英雄图标、模型、动画资源引用
 * - 提供生肖英雄外观剪影、动画基调
 *
 * 固定十二生肖展示命名数据：
 * - Zodiac.Rat：夜隐灵鼠
 * - Zodiac.Ox：镇岳神牛
 * - Zodiac.Tiger：裂风虎君
 * - Zodiac.Rabbit：月华灵兔
 * - Zodiac.Dragon：云巡龙君
 * - Zodiac.Snake：玄花灵蛇
 * - Zodiac.Horse：踏风天驹
 * - Zodiac.Goat：灵泽仙羊
 * - Zodiac.Monkey：幻云灵猿
 * - Zodiac.Rooster：曜鸣神鸡
 * - Zodiac.Dog：镇魄灵犬
 * - Zodiac.Pig：福岳灵猪
 *
 * 注意：
 * - 生肖决定英雄身份、外观剪影、动画基调、生肖大招来源
 * - 生肖与自然元素之力，五大阵营解耦
 * - 生肖与 TeamId 解耦
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAZodiacHeroDisplayRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 生肖枚举值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Hero")
	EDBAZodiac Zodiac = EDBAZodiac::None;

	/** 生肖英雄中文名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Hero")
	FText DisplayName;

	/** 生肖英雄英文名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Hero")
	FString EnglishName;

	/** 生肖英雄简短描述 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Hero")
	FText ShortDescription;

	/** 生肖英雄详细描述 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Hero")
	FText DetailedDescription;

	/** 生肖英雄背景故事 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Hero")
	FText BackgroundStory;

	/** 生肖英雄图标 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Hero|Visual")
	TSoftObjectPtr<UTexture2D> Icon;

	/** 生肖英雄大图 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Hero|Visual")
	TSoftObjectPtr<UTexture2D> LargeIcon;

	/** 生肖英雄剪影图标 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Hero|Visual")
	TSoftObjectPtr<UTexture2D> SilhouetteIcon;

	/** 生肖英雄肖像 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Hero|Visual")
	TSoftObjectPtr<UTexture2D> Portrait;

	/** 生肖英雄全身立绘 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Hero|Visual")
	TSoftObjectPtr<UTexture2D> FullBodyArt;

	/** 生肖英雄选择界面背景 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Hero|Visual")
	TSoftObjectPtr<UTexture2D> SelectionBackground;

	/** 生肖英雄主题色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Hero|Visual")
	FLinearColor ThemeColor = FLinearColor::White;

	/** 生肖英雄次要色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Hero|Visual")
	FLinearColor SecondaryColor = FLinearColor::White;

	/** 生肖英雄角色定位标签（如：刺客、坦克、法师） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Hero")
	TArray<FName> RoleTags;

	/** 生肖英雄难度等级（1-5） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Hero", meta = (ClampMin = "1", ClampMax = "5"))
	int32 DifficultyLevel = 3;

	/** 生肖英雄推荐等级 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Hero")
	int32 RecommendedLevel = 1;

	/** 排序权重 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Hero")
	int32 SortOrder = 0;

	/** 是否可用 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Hero")
	bool bIsAvailable = true;

	/** 是否需要解锁 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Hero")
	bool bRequiresUnlock = false;

	/** 解锁条件描述 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Hero")
	FText UnlockCondition;

	/** 是否在开发中 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Hero")
	bool bIsInDevelopment = false;

	FDBAZodiacHeroDisplayRow()
	{
	}
};

/**
 * 生肖英雄配置数据行
 *
 * 用途：
 * - 定义生肖英雄的游戏配置
 * - 提供生肖英雄基础属性
 * - 提供生肖英雄资源引用（模型、动画、音效）
 * - 提供生肖大招配置
 *
 * 注意：
 * - ZodiacUltimate 是生肖大招
 * - 每个生肖必须有一个 ZodiacUltimate 语义
 * - ZodiacUltimate 输入槽为 Ultimate
 * - ZodiacUltimate 消耗 UltimateEnergy
 * - ZodiacUltimate 的自然元素之力属性由当前 PrimaryElement 决定
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAZodiacHeroConfigRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 生肖枚举值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Config")
	EDBAZodiac Zodiac = EDBAZodiac::None;

	/** 生肖大招技能 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Config|Ultimate")
	FName ZodiacUltimateSkillId;

	/** 生肖大招名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Config|Ultimate")
	FText UltimateName;

	/** 生肖大招描述 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Config|Ultimate")
	FText UltimateDescription;

	/** 生肖大招图标 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Config|Ultimate")
	TSoftObjectPtr<UTexture2D> UltimateIcon;

	/** 生肖大招消耗的终极能量 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Config|Ultimate")
	float UltimateEnergyCost = 100.0f;

	/** 生肖大招冷却时间（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Config|Ultimate")
	float UltimateCooldown = 120.0f;

	/** 基础最大生命值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Config|Attributes")
	float BaseMaxHealth = 1000.0f;

	/** 基础攻击力 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Config|Attributes")
	float BaseAttackPower = 100.0f;

	/** 基础防御力 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Config|Attributes")
	float BaseDefense = 50.0f;

	/** 基础移动速度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Config|Attributes")
	float BaseMoveSpeed = 600.0f;

	/** 基础最大能量 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Config|Attributes")
	float BaseMaxEnergy = 100.0f;

	/** 基础能量恢复速度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Config|Attributes")
	float BaseEnergyRegen = 5.0f;

	/** 基础暴击率 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Config|Attributes")
	float BaseCriticalRate = 0.1f;

	/** 基础暴击倍率 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Config|Attributes")
	float BaseCriticalMultiplier = 2.0f;

	/** 角色类路径 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Config|Resources")
	FSoftClassPath CharacterClass;

	/** 骨骼网格体 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Config|Resources")
	TSoftObjectPtr<USkeletalMesh> SkeletalMesh;

	/** 动画蓝图类 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Config|Resources")
	FSoftClassPath AnimBlueprintClass;

	/** 音效集合 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Config|Resources")
	TSoftObjectPtr<USoundCue> VoiceSoundCue;

	/** 特效集合 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Config|Resources")
	TSoftObjectPtr<UParticleSystem> EffectSystem;

	/** 是否可用 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zodiac|Config")
	bool bIsAvailable = true;

	FDBAZodiacHeroConfigRow()
	{
	}
};
