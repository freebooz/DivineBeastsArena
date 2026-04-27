// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ArenaTestGameState.generated.h"

/**
 * ArenaTestGameState
 *
 * 测试用 GameState
 * 管理游戏状态和玩家信息
 */
UCLASS()
class DIVINEBEASTSARENA_API AArenaTestGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AArenaTestGameState();

	virtual void BeginPlay() override;

public:
	// 当前游戏状态
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Test|GameState")
	FString GameStatus;

	// 测试地图名称
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Test|GameState")
	FString MapName;
};
