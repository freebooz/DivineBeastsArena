// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：声明 GAS 英雄成长属性默认值数据资产，承载可调数值而不承载运行逻辑。
- 修改提示：新增英雄成长属性默认值时，同步更新校验逻辑与 AttributeSet 应用入口。
*/

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Data/DBADataAssetBase.h"
#include "GameDBA/Core/Interfaces/DBAValidatableInterface.h"
#include "DBAHeroGrowthDefaultsDataAsset.generated.h"

/**
 * 英雄成长属性默认值结构体
 * 承载英雄成长相关可调数值，由数据资产驱动，避免硬编码。
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAHeroGrowthDefaults
{
	GENERATED_BODY()

	/** 英雄初始等级（必须大于 0） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|HeroGrowth|Level", meta = (ClampMin = "1.0"))
	float HeroLevel = 1.0f;

	/** 初始经验值（不能为负） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|HeroGrowth|Level", meta = (ClampMin = "0.0"))
	float Experience = 0.0f;

	/** 升至下一级所需经验（必须大于 0） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|HeroGrowth|Level", meta = (ClampMin = "1.0"))
	float ExperienceToNextLevel = 100.0f;

	/** 复活时间（秒，必须大于 0） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|HeroGrowth|State", meta = (ClampMin = "0.0"))
	float RespawnTime = 10.0f;

	/** 击杀奖励金币（不能为负） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DBA|HeroGrowth|State", meta = (ClampMin = "0.0"))
	float GoldBounty = 300.0f;
};

/**
 * 英雄成长属性默认值数据资产
 *
 * 设计依据：
 *   - 项目策略《DBA.DataAsset.NoHardcoding》：成长属性默认值通过 DataAsset 驱动
 *   - 项目策略《DBA.Log.ChineseOutput》：校验失败输出中文错误
 *   - 蓝本：UDBABattleAttributeDefaultsDataAsset
 *
 * 配置方式：
 *   1. 在 Content 编辑器中创建 DA_HeroGrowthDefaults 资产实例
 *   2. 在 DefaultGame.ini 的 DBAHeroGrowthDeveloperSettings 中配置软引用
 */
UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBAHeroGrowthDefaultsDataAsset : public UDBADataAssetBase, public IDBAValidatableInterface
{
	GENERATED_BODY()

public:
	/** 获取英雄成长属性默认值 */
	const FDBAHeroGrowthDefaults& GetDefaults() const { return Defaults; }

	//~ Begin IDBAValidatableInterface Interface
	virtual bool ValidateData_Implementation(TArray<FString>& OutErrors) const override;
	//~ End IDBAValidatableInterface Interface

private:
	/** 英雄成长属性默认值 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|HeroGrowth", meta = (AllowPrivateAccess = "true"))
	FDBAHeroGrowthDefaults Defaults;
};
