// Copyright FreeboozStudio. All Rights Reserved.

#include "Frontend/LoadingPreparation/DBALoadingPreparationSubsystem.h"
#include "Common/DBALogChannels.h"

UDBALoadingPreparationSubsystem::UDBALoadingPreparationSubsystem()
{
}

void UDBALoadingPreparationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LogDBAUI, Log, TEXT("[UDBALoadingPreparationSubsystem] 初始化完成"));
}

void UDBALoadingPreparationSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

void UDBALoadingPreparationSubsystem::InitializeMatchLoadInfo(const FDBA_MATCHLoadInfo& MatchInfo)
{
    CurrentMatchInfo = MatchInfo;
    CurrentMatchInfo.LoadedPlayerCount = 0;
    bIsLoading = true;

    for (FDBAPlayerLoadInfo& Player : CurrentMatchInfo.BlueTeamPlayers)
    {
        Player.bIsLoadComplete = false;
    }

    for (FDBAPlayerLoadInfo& Player : CurrentMatchInfo.RedTeamPlayers)
    {
        Player.bIsLoadComplete = false;
    }

    CurrentMatchInfo.TotalPlayerCount = CurrentMatchInfo.BlueTeamPlayers.Num() + CurrentMatchInfo.RedTeamPlayers.Num();

    OnMatchLoadInfoReady.Broadcast(CurrentMatchInfo);

    UE_LOG(LogDBAUI, Log, TEXT("[UDBALoadingPreparationSubsystem] 初始化比赛加载信息: MatchId=%s, PlayerCount=%d"),
        *MatchInfo.MatchId, CurrentMatchInfo.TotalPlayerCount);
}

void UDBALoadingPreparationSubsystem::UpdatePlayerLoadProgress(const FString& PlayerId, bool bLoadComplete)
{
    if (!bIsLoading)
    {
        return;
    }

    bool bFound = false;

    for (FDBAPlayerLoadInfo& Player : CurrentMatchInfo.BlueTeamPlayers)
    {
        if (Player.PlayerId == PlayerId)
        {
            Player.bIsLoadComplete = bLoadComplete;
            bFound = true;
            break;
        }
    }

    if (!bFound)
    {
        for (FDBAPlayerLoadInfo& Player : CurrentMatchInfo.RedTeamPlayers)
        {
            if (Player.PlayerId == PlayerId)
            {
                Player.bIsLoadComplete = bLoadComplete;
                bFound = true;
                break;
            }
        }
    }

    if (bFound)
    {
        CurrentMatchInfo.LoadedPlayerCount = 0;
        for (const FDBAPlayerLoadInfo& Player : CurrentMatchInfo.BlueTeamPlayers)
        {
            if (Player.bIsLoadComplete)
            {
                CurrentMatchInfo.LoadedPlayerCount++;
            }
        }
        for (const FDBAPlayerLoadInfo& Player : CurrentMatchInfo.RedTeamPlayers)
        {
            if (Player.bIsLoadComplete)
            {
                CurrentMatchInfo.LoadedPlayerCount++;
            }
        }

        float Progress = GetOverallLoadProgress();
        OnPlayerLoadProgress.Broadcast(Progress);

        UE_LOG(LogDBAUI, Log, TEXT("[UDBALoadingPreparationSubsystem] 玩家加载进度: %s, %d/%d"),
            *PlayerId, CurrentMatchInfo.LoadedPlayerCount, CurrentMatchInfo.TotalPlayerCount);

        if (AreAllPlayersLoaded())
        {
            bIsLoading = false;
            OnAllPlayersLoaded.Broadcast(true);
        }
    }
}

float UDBALoadingPreparationSubsystem::GetOverallLoadProgress() const
{
    if (CurrentMatchInfo.TotalPlayerCount == 0)
    {
        return 1.0f;
    }

    return static_cast<float>(CurrentMatchInfo.LoadedPlayerCount) / static_cast<float>(CurrentMatchInfo.TotalPlayerCount);
}

bool UDBALoadingPreparationSubsystem::AreAllPlayersLoaded() const
{
    return CurrentMatchInfo.LoadedPlayerCount >= CurrentMatchInfo.TotalPlayerCount;
}

void UDBALoadingPreparationSubsystem::StartPostLoading()
{
    UE_LOG(LogDBAUI, Log, TEXT("[UDBALoadingPreparationSubsystem] 开始后加载阶段"));
}

void UDBALoadingPreparationSubsystem::CancelLoading()
{
    bIsLoading = false;
    CurrentMatchInfo = FDBA_MATCHLoadInfo();

    UE_LOG(LogDBAUI, Log, TEXT("[UDBALoadingPreparationSubsystem] 取消加载"));
}
