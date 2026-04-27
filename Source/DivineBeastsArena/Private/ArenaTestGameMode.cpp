// Copyright FreeboozStudio. All Rights Reserved.

#include "ArenaTestGameMode.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Common/DBALogChannels.h"

AArenaTestGameMode::AArenaTestGameMode()
	: ElapsedTime(0.0f)
{
	// 允许暂停
	bPauseable = true;
}

void AArenaTestGameMode::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogDBACombat, Log, TEXT("[ArenaTestGameMode] 开始游戏"));
}

void AArenaTestGameMode::StartPlay()
{
	Super::StartPlay();

	UE_LOG(LogDBACombat, Log, TEXT("[ArenaTestGameMode] 开始游戏 - 比赛时长: %f"), MatchDuration);

	// 查找所有 PlayerStart
	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);

	UE_LOG(LogDBACombat, Log, TEXT("[ArenaTestGameMode] 找到 %d 个玩家起始点"), PlayerStarts.Num());

	for (AActor* PlayerStart : PlayerStarts)
	{
		PlayerStartLocations.Add(PlayerStart->GetActorLocation());
	}
}

void AArenaTestGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ElapsedTime += DeltaTime;

	// 超时后结束游戏
	if (MatchDuration > 0.0f && ElapsedTime >= MatchDuration)
	{
		UE_LOG(LogDBACombat, Log, TEXT("[ArenaTestGameMode] 比赛时间已超时"));
	}
}
