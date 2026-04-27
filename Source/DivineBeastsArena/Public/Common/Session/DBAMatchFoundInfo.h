// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/Types/DBACommonEnums.h"
#include "DBAMatchFoundInfo.generated.h"

/**
 * 匹配信息结构体
 * 包含一场匹配的基本信息
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAMatchFoundInfo
{
	GENERATED_BODY()

	/** 匹配ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match")
	FString MatchId;

	/** 地图名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match")
	FString MapName;

	/** 游戏模式 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match")
	FString GameMode;

	/** 平均等级 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match")
	int32 AverageLevel = 1;

	/** 准备确认超时时间（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Match")
	float ReadyCheckTimeout = 30.0f;

	FDBAMatchFoundInfo()
		: AverageLevel(1)
		, ReadyCheckTimeout(30.0f)
	{
	}
};
