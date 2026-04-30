// Copyright Freebooz Games, Inc. All Rights Reserved.
// 神兽竞技场 - MOBA GameState 基类实现

#include "MobaBase/Framework/DBAGameStateBase.h"
#include "Common/DBALogChannels.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

ADBAMobaGameStateBase::ADBAMobaGameStateBase()
{
    // 启用复制
    bReplicates = true;
    // UE 5.7: 使用 SetNetUpdateFrequency 替代直接赋值
    SetNetUpdateFrequency(1.0f);
}

void ADBAMobaGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ADBAMobaGameStateBase, CurrentGameState);
}

void ADBAMobaGameStateBase::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogDBACombat, Log, TEXT("[ADBAMobaGameStateBase] BeginPlay"));
}

float ADBAMobaGameStateBase::GetElapsedTime() const
{
    return GetWorld()->GetTimeSeconds();
}

float ADBAMobaGameStateBase::GetMatchDurationLimit() const
{
    return MatchDurationLimit;
}

float ADBAMobaGameStateBase::GetRemainingTime() const
{
    if (MatchDurationLimit <= 0.0f)
    {
        return 0.0f;
    }

    const float Elapsed = GetElapsedTime();
    const float Remaining = MatchDurationLimit - Elapsed;
    return FMath::Max(Remaining, 0.0f);
}

int32 ADBAMobaGameStateBase::GetTeamCount() const
{
    // 默认两个队伍
    return 2;
}

int32 ADBAMobaGameStateBase::GetPlayerCountByTeam(int32 TeamId) const
{
    int32 Count = 0;
    for (APlayerState* PlayerState : PlayerArray)
    {
        if (PlayerState)
        {
            Count++;
        }
    }
    return Count;
}

TArray<APlayerState*> ADBAMobaGameStateBase::GetAllPlayerStates() const
{
    return PlayerArray;
}