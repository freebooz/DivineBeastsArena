// Copyright Freebooz Games, Inc. All Rights Reserved.
// 神兽竞技场 - 十二生肖数值平衡配置

#pragma once

#include "CoreMinimal.h"
#include "GameDBA/Core/DBAEnumsCore.h"
#include "DBAAbilityBalance.generated.h"

/**
 * FDBAHeroCoreStats
 * 英雄核心能力评分
 * 用于UI展示和匹配系统
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAHeroCoreStats
{
	GENERATED_BODY()

	/** 生存能力 (1-5) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 Survivability = 3;

	/** 伤害能力 (1-5) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 Damage = 3;

	/** 控制能力 (1-5) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 Control = 3;

	/** 机动能力 (1-5) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 Mobility = 3;

	/** 辅助能力 (1-5) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 Support = 3;

	/** 操作难度 (1-5) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 Difficulty = 3;

	/** 团战影响 (1-5) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 TeamFightImpact = 3;
};

/**
 * FDBAHeroPositionInfo
 * 英雄分路信息
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAHeroPositionInfo
{
	GENERATED_BODY()

	/** 推荐分路 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString RecommendedLane;

	/** 团队职责 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString TeamRole;
};

/**
 * FDBAHeroBalanceData
 * 单个英雄的数值平衡数据
 * 包含评分、分路信息、优劣势
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAHeroBalanceData
{
	GENERATED_BODY()

	/** 生肖类型 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EDBAZodiacType ZodiacType = EDBAZodiacType::None;

	/** 角色全称 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString CharacterName;

	/** 短名 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString ShortName;

	/** 核心定位 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString CoreRole;

	/** 核心能力评分 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FDBAHeroCoreStats CoreStats;

	/** 分路信息 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FDBAHeroPositionInfo PositionInfo;

	/** 主要优势 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString Advantages;

	/** 明显短板 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString Weaknesses;

	/** 最佳搭档 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString BestPartners;
};

/**
 * UDBAHeroBalanceConfig
 * 英雄数值平衡配置表
 * 所有12个生肖英雄的平衡数据
 */
UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBAHeroBalanceConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UDBAHeroBalanceConfig();

	/** 获取指定生肖的英雄数据 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Balance")
	FDBAHeroBalanceData GetHeroBalanceData(EDBAZodiacType ZodiacType) const;

	/** 获取所有英雄数据 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Balance")
	TArray<FDBAHeroBalanceData> GetAllHeroBalanceData() const;

public:
	/** 子鼠·夜影灵牙｜影牙 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FDBAHeroBalanceData Rat_ShadowFang;

	/** 丑牛·撼山铁角｜铁角 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FDBAHeroBalanceData Ox_Ironhorn;

	/** 寅虎·啸山白虎｜白虎 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FDBAHeroBalanceData Tiger_WhiteTiger;

	/** 卯兔·踏月玉灵｜玉灵 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FDBAHeroBalanceData Rabbit_MoonSpirit;

	/** 辰龙·御雷苍龙｜苍龙 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FDBAHeroBalanceData Dragon_ThunderLord;

	/** 巳蛇·幽毒灵蛇｜幽鳞 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FDBAHeroBalanceData Snake_VenomScale;

	/** 午马·赤焰雷蹄｜雷蹄 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FDBAHeroBalanceData Horse_ThunderHoof;

	/** 未羊·玉角灵铃｜玉角 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FDBAHeroBalanceData Goat_JadeBell;

	/** 申猴·百戏灵猴｜灵猴 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FDBAHeroBalanceData Monkey_Trickster;

	/** 酉鸡·破晓金翎｜金翎 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FDBAHeroBalanceData Rooster_DawnBringer;

	/** 戌狗·守门天犬｜天犬 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FDBAHeroBalanceData Dog_SkyGuardian;

	/** 亥猪·岩甲獠牙｜獠牙 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FDBAHeroBalanceData Pig_StoneTusk;
};
