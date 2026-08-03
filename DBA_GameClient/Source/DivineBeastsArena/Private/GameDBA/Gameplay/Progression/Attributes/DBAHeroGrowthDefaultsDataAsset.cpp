// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：实现 GAS 英雄成长属性默认值数据资产的数据完整性校验。
- 修改提示：校验只检查数据合法性，不写入运行时属性，不访问外部服务。
*/

#include "GameDBA/Gameplay/Progression/Attributes/DBAHeroGrowthDefaultsDataAsset.h"

namespace
{
	/** 追加一条中文校验错误 */
	void AddHeroGrowthDefaultError(TArray<FString>& OutErrors, const TCHAR* FieldName, const TCHAR* Reason)
	{
		OutErrors.Add(FString::Printf(TEXT("英雄成长属性默认值校验失败：字段=%s 原因=%s"), FieldName, Reason));
	}
}

bool UDBAHeroGrowthDefaultsDataAsset::ValidateData_Implementation(TArray<FString>& OutErrors) const
{
	const int32 OriginalErrorCount = OutErrors.Num();

	// 英雄等级必须大于等于 1
	if (Defaults.HeroLevel < 1.0f)
	{
		AddHeroGrowthDefaultError(OutErrors, TEXT("HeroLevel"), TEXT("英雄初始等级必须大于等于 1"));
	}

	// 经验值不能为负
	if (Defaults.Experience < 0.0f)
	{
		AddHeroGrowthDefaultError(OutErrors, TEXT("Experience"), TEXT("初始经验值不能为负数"));
	}

	// 升级所需经验必须大于 0
	if (Defaults.ExperienceToNextLevel <= 0.0f)
	{
		AddHeroGrowthDefaultError(OutErrors, TEXT("ExperienceToNextLevel"), TEXT("升至下一级所需经验必须大于 0"));
	}

	// 复活时间必须大于等于 0
	if (Defaults.RespawnTime < 0.0f)
	{
		AddHeroGrowthDefaultError(OutErrors, TEXT("RespawnTime"), TEXT("复活时间不能为负数"));
	}

	// 击杀奖励金币不能为负
	if (Defaults.GoldBounty < 0.0f)
	{
		AddHeroGrowthDefaultError(OutErrors, TEXT("GoldBounty"), TEXT("击杀奖励金币不能为负数"));
	}

	return OutErrors.Num() == OriginalErrorCount;
}
