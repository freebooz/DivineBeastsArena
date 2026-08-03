// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

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
