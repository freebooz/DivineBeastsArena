// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：声明英雄数值平衡数据的 DataTable 行结构体，用于 CSV 驱动的英雄平衡配置。
- 阅读重点：FDBAHeroBalanceRow 是扁平化的 CSV 友好结构，ToHeroBalanceData() 转换回 FDBAHeroBalanceData 供业务使用。
- 修改提示：新增字段时需同步更新 CSV 源文件和 ToHeroBalanceData() 转换逻辑。
*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameDBA/Core/DBAEnumsCore.h"
#include "GameDBA/Gameplay/Progression/Balance/DBAAbilityBalance.h"
#include "DBAHeroBalanceRow.generated.h"

/**
 * FDBAHeroBalanceRow
 *
 * 英雄数值平衡 DataTable 行结构（扁平化，CSV 友好）。
 *
 * 设计原因：
 * - 旧版 CDO 以字段形式存储 12 条记录，扩展性差且未接入运行时加载，现已清理。
 * - 本结构扁平化了 FDBAHeroCoreStats 和 FDBAHeroPositionInfo 的字段，便于 CSV 直接导入。
 * - 通过 ToHeroBalanceData() 转换回业务侧使用的 FDBAHeroBalanceData，保持向后兼容。
 *
 * CSV 列顺序：
 * RowName,ZodiacType,CharacterName,ShortName,CoreRole,Survivability,Damage,Control,Mobility,Support,Difficulty,TeamFightImpact,RecommendedLane,TeamRole,Advantages,Weaknesses,BestPartners
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAHeroBalanceRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	/** 生肖类型（int 形式，对应 EDBAZodiacType 枚举值） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Balance")
	int32 ZodiacType = 0;

	/** 角色全称 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Balance")
	FString CharacterName;

	/** 短名 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Balance")
	FString ShortName;

	/** 核心定位 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Balance")
	FString CoreRole;

	/** 生存能力 (1-5) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Balance|CoreStats", meta = (ClampMin = "1", ClampMax = "5"))
	int32 Survivability = 3;

	/** 伤害能力 (1-5) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Balance|CoreStats", meta = (ClampMin = "1", ClampMax = "5"))
	int32 Damage = 3;

	/** 控制能力 (1-5) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Balance|CoreStats", meta = (ClampMin = "1", ClampMax = "5"))
	int32 Control = 3;

	/** 机动能力 (1-5) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Balance|CoreStats", meta = (ClampMin = "1", ClampMax = "5"))
	int32 Mobility = 3;

	/** 辅助能力 (1-5) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Balance|CoreStats", meta = (ClampMin = "1", ClampMax = "5"))
	int32 Support = 3;

	/** 操作难度 (1-5) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Balance|CoreStats", meta = (ClampMin = "1", ClampMax = "5"))
	int32 Difficulty = 3;

	/** 团战影响 (1-5) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Balance|CoreStats", meta = (ClampMin = "1", ClampMax = "5"))
	int32 TeamFightImpact = 3;

	/** 推荐分路 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Balance|Position")
	FString RecommendedLane;

	/** 团队职责 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Balance|Position")
	FString TeamRole;

	/** 主要优势 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Balance")
	FString Advantages;

	/** 明显短板 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Balance")
	FString Weaknesses;

	/** 最佳搭档 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|Balance")
	FString BestPartners;

public:
	/** 转换为业务侧使用的 FDBAHeroBalanceData 结构体。 */
	FDBAHeroBalanceData ToHeroBalanceData() const
	{
		FDBAHeroBalanceData Result;
		Result.ZodiacType = static_cast<EDBAZodiacType>(ZodiacType);
		Result.CharacterName = CharacterName;
		Result.ShortName = ShortName;
		Result.CoreRole = CoreRole;
		Result.CoreStats = FDBAHeroCoreStats{Survivability, Damage, Control, Mobility, Support, Difficulty, TeamFightImpact};
		Result.PositionInfo = FDBAHeroPositionInfo{RecommendedLane, TeamRole};
		Result.Advantages = Advantages;
		Result.Weaknesses = Weaknesses;
		Result.BestPartners = BestPartners;
		return Result;
	}

	/** 从业务侧 FDBAHeroBalanceData 填充本结构体。 */
	void FromHeroBalanceData(const FDBAHeroBalanceData& Data)
	{
		ZodiacType = static_cast<int32>(Data.ZodiacType);
		CharacterName = Data.CharacterName;
		ShortName = Data.ShortName;
		CoreRole = Data.CoreRole;
		Survivability = Data.CoreStats.Survivability;
		Damage = Data.CoreStats.Damage;
		Control = Data.CoreStats.Control;
		Mobility = Data.CoreStats.Mobility;
		Support = Data.CoreStats.Support;
		Difficulty = Data.CoreStats.Difficulty;
		TeamFightImpact = Data.CoreStats.TeamFightImpact;
		RecommendedLane = Data.PositionInfo.RecommendedLane;
		TeamRole = Data.PositionInfo.TeamRole;
		Advantages = Data.Advantages;
		Weaknesses = Data.Weaknesses;
		BestPartners = Data.BestPartners;
	}
};

/**
 * 根据生肖类型构建 DataTable 行名。
 *
 * 行名规则：使用生肖英文名（Rat/Ox/Tiger/Rabbit/Dragon/Snake/Horse/Goat/Monkey/Rooster/Dog/Pig），
 * 与 EDBAZodiacType 枚举顺序一致，便于通过 ZodiacType 直接查询。
 *
 * @param ZodiacType 生肖类型
 * @return 行名（如 "Rat"、"Ox"）
 */
DIVINEBEASTSARENA_API FName BuildHeroBalanceRowName(EDBAZodiacType ZodiacType);
