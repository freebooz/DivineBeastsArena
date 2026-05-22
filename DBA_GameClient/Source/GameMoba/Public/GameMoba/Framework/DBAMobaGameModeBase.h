// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// GameMoba - 通用MOBA GameMode基类

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DBAMobaGameModeBase.generated.h"

/**
 * ADBAMobaGameModeBase
 * MOBA游戏通用GameMode基类
 * 提供游戏模式通用接口
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class GAMEMOBA_API ADBAMobaGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ADBAMobaGameModeBase();

protected:
	//~ Begin AGameModeBase Interface
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void InitGameState() override;
	virtual void StartMatch();
	virtual void HandleMatchHasStarted();
	virtual void HandleMatchHasEnded();
	//~ End AGameModeBase Interface

protected:
	/** 游戏是否已开始 */
	UPROPERTY(BlueprintReadOnly, Category = "DBA|GameMode|Base")
	bool bMatchStarted = false;

	/** 游戏是否结束 */
	UPROPERTY(BlueprintReadOnly, Category = "DBA|GameMode|Base")
	bool bMatchEnded = false;

	/** 游戏开始时间 */
	UPROPERTY()
	float MatchStartTime = 0.0f;

	/** 游戏结束时间 */
	UPROPERTY()
	float MatchEndTime = 0.0f;
};