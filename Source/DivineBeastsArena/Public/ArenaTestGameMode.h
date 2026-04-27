// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ArenaTestGameMode.generated.h"

/**
 * ArenaTestGameMode
 *
 * 测试用 GameMode
 * 提供基本的游戏规则和初始化
 */
UCLASS()
class DIVINEBEASTSARENA_API AArenaTestGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AArenaTestGameMode();

	virtual void StartPlay() override;
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

public:
	// 测试用玩家出生点
	UPROPERTY(EditDefaultsOnly, Category = "Test")
	TArray<FVector> PlayerStartLocations;

	// 测试用回合时间
	UPROPERTY(EditDefaultsOnly, Category = "Test")
	float MatchDuration = 600.0f;

	// 当前已用时间
	float ElapsedTime;
};
