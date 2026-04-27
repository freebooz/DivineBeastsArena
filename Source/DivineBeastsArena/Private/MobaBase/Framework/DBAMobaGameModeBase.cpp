// Copyright Epic Games, Inc. All Rights Reserved.
// 神兽竞技场 - MOBA GameMode 基类实现

#include "MobaBase/Framework/DBAMobaGameModeBase.h"
#include "Common/DBALogChannels.h"

ADBAMobaGameModeBase::ADBAMobaGameModeBase()
{
    // 基础配置
    bPauseable = true;
}

void ADBAMobaGameModeBase::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogDBACombat, Log, TEXT("ADBAMobaGameModeBase: BeginPlay"));
}

void ADBAMobaGameModeBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UE_LOG(LogDBACombat, Log, TEXT("ADBAMobaGameModeBase: EndPlay"));

    Super::EndPlay(EndPlayReason);
}

void ADBAMobaGameModeBase::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    UE_LOG(LogDBACombat, Log, TEXT("ADBAMobaGameModeBase: PostLogin - Player: %s"), *NewPlayer->GetName());
}

void ADBAMobaGameModeBase::Logout(AController* Exiting)
{
    UE_LOG(LogDBACombat, Log, TEXT("ADBAMobaGameModeBase: Logout - Controller: %s"), *Exiting->GetName());

    Super::Logout(Exiting);
}

bool ADBAMobaGameModeBase::Pause()
{
    return false;
}

bool ADBAMobaGameModeBase::UnPause()
{
    return false;
}

bool ADBAMobaGameModeBase::IsGameRunning() const
{
    return false;
}

int32 ADBAMobaGameModeBase::GetCurrentPlayerCount() const
{
    return GetWorld()->GetGameState()->PlayerArray.Num();
}

int32 ADBAMobaGameModeBase::GetMaxPlayerCount() const
{
    return 10;
}

void ADBAMobaGameModeBase::OnPlayerCountChanged_Implementation(int32 OldCount, int32 NewCount)
{
    UE_LOG(LogDBACombat, Log, TEXT("ADBAMobaGameModeBase: PlayerCount changed from %d to %d"), OldCount, NewCount);
}

void ADBAMobaGameModeBase::OnGameStateChanged_Implementation(EDBAGameModeState NewState)
{
    UE_LOG(LogDBACombat, Log, TEXT("ADBAMobaGameModeBase: GameState changed to %s"), *UEnum::GetValueAsString(NewState));
}