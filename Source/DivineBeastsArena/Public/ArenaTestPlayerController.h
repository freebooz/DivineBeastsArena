// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ArenaTestPlayerController.generated.h"

/**
 * ArenaTestPlayerController
 *
 * 测试用 PlayerController
 * 提供基本的玩家输入处理
 */
UCLASS()
class DIVINEBEASTSARENA_API AArenaTestPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AArenaTestPlayerController();

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

public:
	// 测试用玩家名称
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Test|Player")
	FString PlayerDisplayName;
};
