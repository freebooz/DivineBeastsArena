// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// 伤害计算 Value Objects

#pragma once

#include "CoreMinimal.h"
#include "GameDBA/Core/DBAEnumsCore.h"
#include "DBADamageTypes.generated.h"

/**
 * FDamageCalculationParams
 * 伤害计算参数 - 输入
 * 封装所有伤害计算所需的输入参数
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDamageCalculationParams
{
	GENERATED_BODY()

	/** 基础伤害 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float BaseDamage = 0.0f;

	/** 攻击方元素 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	EDBAElementType AttackElement = EDBAElementType::None;

	/** 防御方元素 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	EDBAElementType DefenseElement = EDBAElementType::None;

	/** 共鸣等级 (0-4) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 ResonanceLevel = 0;

	/** 连锁等级 (0-10) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 ChainLevel = 0;

	/** 防御力 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float Defense = 0.0f;

	/** 暴击率 (0-1) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float CriticalRate = 0.0f;

	/** 暴击倍率 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float CriticalMultiplier = 2.0f;
};

/**
 * FElementCounterResult
 * 元素克制结果
 * 封装元素克制计算的输出结果
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FElementCounterResult
{
	GENERATED_BODY()

	/** 克制倍率 */
	UPROPERTY(BlueprintReadOnly)
	float Multiplier = 1.0f;

	/** 克制类型 */
	UPROPERTY(BlueprintReadOnly)
	EDBAElementCounterResult ResultType = EDBAElementCounterResult::None;
};

/**
 * FChainBonus
 * 连锁加成
 * 封装连锁系统伤害加成
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FChainBonus
{
	GENERATED_BODY()

	/** 连锁倍率 */
	UPROPERTY(BlueprintReadOnly)
	float Multiplier = 1.0f;

	/** 是否为终结连锁 */
	UPROPERTY(BlueprintReadOnly)
	bool bIsFinal = false;
};

/**
 * FFinalDamageResult
 * 最终伤害结果 - 输出
 * 封装完整伤害计算的所有输出
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FFinalDamageResult
{
	GENERATED_BODY()

	/** 计算后伤害值 */
	UPROPERTY(BlueprintReadOnly)
	float FinalDamage = 0.0f;

	/** 是否暴击 */
	UPROPERTY(BlueprintReadOnly)
	bool bIsCritical = false;

	/** 元素克制结果 */
	UPROPERTY(BlueprintReadOnly)
	FElementCounterResult ElementResult;

	/** 连锁加成 */
	UPROPERTY(BlueprintReadOnly)
	FChainBonus ChainBonus;

	/** 共鸣加成百分比 */
	UPROPERTY(BlueprintReadOnly)
	float ResonanceBonusPercent = 0.0f;

	/** 防御减免百分比 */
	UPROPERTY(BlueprintReadOnly)
	float DefenseReductionPercent = 0.0f;
};
