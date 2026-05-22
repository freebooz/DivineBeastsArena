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
#include "GameCore/Types/DBACommonEnums.h"
#include "DBAMatchFoundInfo.generated.h"

/**
 * 匹配信息结构体
 * 包含一场匹配的基本信息
 */
USTRUCT(BlueprintType)
struct GAMECORE_API FDBAMatchFoundInfo
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
