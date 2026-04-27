// Copyright FreeboozStudio. All Rights Reserved.

#include "ArenaTestPlayerController.h"
#include "Engine/World.h"
#include "Common/DBALogChannels.h"

AArenaTestPlayerController::AArenaTestPlayerController()
{
	// 启用复制
	bReplicates = true;
}

void AArenaTestPlayerController::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogDBACombat, Log, TEXT("[ArenaTestPlayerController] 开始游戏"));
}

void AArenaTestPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
}

void AArenaTestPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (UWorld* World = GetWorld())
	{
		UE_LOG(LogDBACombat, Log, TEXT("[ArenaTestPlayerController] 控制角色: %s"), *InPawn->GetName());
	}
}

void AArenaTestPlayerController::OnUnPossess()
{
	Super::OnUnPossess();

	UE_LOG(LogDBACombat, Log, TEXT("[ArenaTestPlayerController] 释放控制权"));
}
