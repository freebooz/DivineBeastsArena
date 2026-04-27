// Copyright FreeboozStudio. All Rights Reserved.

#include "ArenaTestGameState.h"
#include "Engine/World.h"
#include "Common/DBALogChannels.h"

AArenaTestGameState::AArenaTestGameState()
	: GameStatus(TEXT("Initializing"))
{
	// 启用复制
	bReplicates = true;
}

void AArenaTestGameState::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		MapName = World->GetMapName();
	}

	UE_LOG(LogDBACombat, Log, TEXT("[ArenaTestGameState] 开始游戏 - 地图: %s"), *MapName);
}
