// Copyright Freebooz Games, Inc. All Rights Reserved.
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
	virtual void StartMatch() override;
	virtual void HandleMatchHasStarted() override;
	virtual void HandleMatchHasEnded() override;
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