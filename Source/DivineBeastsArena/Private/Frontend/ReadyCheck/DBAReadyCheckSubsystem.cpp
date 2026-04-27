// Copyright FreeboozStudio. All Rights Reserved.

#include "Frontend/ReadyCheck/DBAReadyCheckSubsystem.h"
#include "Common/DBALogChannels.h"
#include "GameFramework/PlayerState.h"
#include "TimerManager.h"

UDBAReadyCheckSubsystem::UDBAReadyCheckSubsystem()
{
}

void UDBAReadyCheckSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LogDBAUI, Log, TEXT("[UDBAReadyCheckSubsystem] 初始化完成"));
}

void UDBAReadyCheckSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(RemainingTimeTimerHandle);
        World->GetTimerManager().ClearTimer(TimeoutTimerHandle);
    }

    Super::Deinitialize();
}

void UDBAReadyCheckSubsystem::StartReadyCheck(const FString& MatchId, float TimeoutSeconds)
{
    if (IsInReadyCheck())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[UDBAReadyCheckSubsystem] 已在 ReadyCheck 中"));
        return;
    }

    CurrentReadyCheckInfo.ReadyCheckId = FGuid::NewGuid().ToString();
    CurrentReadyCheckInfo.MatchId = MatchId;
    CurrentReadyCheckInfo.TimeoutSeconds = TimeoutSeconds;
    CurrentReadyCheckInfo.RemainingTime = TimeoutSeconds;
    CurrentReadyCheckInfo.StartTime = FDateTime::Now();
    CurrentReadyCheckInfo.Players.Empty();

    FString LocalPlayerId = GetLocalPlayerId();
    for (int32 i = 0; i < 10; i++)
    {
        FDBAReadyCheckSubsystemPlayerInfo PlayerInfo;
        PlayerInfo.PlayerId = FString::Printf(TEXT("Player_%d"), i);
        PlayerInfo.DisplayName = FString::Printf(TEXT("玩家 %d"), i + 1);
        PlayerInfo.State = EDBAReadyCheckPlayerState::Pending;
        PlayerInfo.bIsLocalPlayer = (PlayerInfo.PlayerId == LocalPlayerId);
        CurrentReadyCheckInfo.Players.Add(PlayerInfo);
    }

    SetReadyCheckState(EDBAReadyCheckSubsystemState::WaitingForPlayers);

    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().SetTimer(
            RemainingTimeTimerHandle,
            this,
            &UDBAReadyCheckSubsystem::UpdateRemainingTime,
            RemainingTimeUpdateFrequency,
            true
        );

        World->GetTimerManager().SetTimer(
            TimeoutTimerHandle,
            this,
            &UDBAReadyCheckSubsystem::HandleReadyCheckTimeout,
            TimeoutSeconds,
            false
        );
    }

    UE_LOG(LogDBAUI, Log, TEXT("[UDBAReadyCheckSubsystem] 开始 ReadyCheck: MatchId=%s, Timeout=%.1f"), *MatchId, TimeoutSeconds);
}

void UDBAReadyCheckSubsystem::AcceptReadyCheck()
{
    if (!IsInReadyCheck())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[UDBAReadyCheckSubsystem] 当前不在 ReadyCheck 中"));
        return;
    }

    FString LocalPlayerId = GetLocalPlayerId();
    UpdatePlayerState(LocalPlayerId, EDBAReadyCheckPlayerState::Ready);

    UE_LOG(LogDBAUI, Log, TEXT("[UDBAReadyCheckSubsystem] 确认准备"));

    CheckReadyCheckCompletion();
}

void UDBAReadyCheckSubsystem::DeclineReadyCheck()
{
    if (!IsInReadyCheck())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[UDBAReadyCheckSubsystem] 当前不在 ReadyCheck 中"));
        return;
    }

    FString LocalPlayerId = GetLocalPlayerId();
    UpdatePlayerState(LocalPlayerId, EDBAReadyCheckPlayerState::Declined);

    UE_LOG(LogDBAUI, Log, TEXT("[UDBAReadyCheckSubsystem] 拒绝准备"));

    SetReadyCheckState(EDBAReadyCheckSubsystemState::Declined);
    OnReadyCheckCompleted.Broadcast(false);

    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(RemainingTimeTimerHandle);
        World->GetTimerManager().ClearTimer(TimeoutTimerHandle);
    }

    CurrentReadyCheckInfo = FDBAReadyCheckSubsystemInfo();
}

void UDBAReadyCheckSubsystem::SetReadyCheckState(EDBAReadyCheckSubsystemState NewState)
{
    if (CurrentReadyCheckInfo.State != NewState)
    {
        CurrentReadyCheckInfo.State = NewState;
        OnReadyCheckStateChanged.Broadcast(NewState);
        UE_LOG(LogDBAUI, Log, TEXT("[UDBAReadyCheckSubsystem] ReadyCheck 状态变更: %d"), static_cast<int32>(NewState));
    }
}

void UDBAReadyCheckSubsystem::UpdatePlayerState(const FString& PlayerId, EDBAReadyCheckPlayerState NewState)
{
    for (FDBAReadyCheckSubsystemPlayerInfo& Player : CurrentReadyCheckInfo.Players)
    {
        if (Player.PlayerId == PlayerId)
        {
            if (Player.State != NewState)
            {
                Player.State = NewState;
                OnReadyCheckPlayerStateChanged.Broadcast(PlayerId, NewState);
            }
            break;
        }
    }
}

void UDBAReadyCheckSubsystem::UpdateRemainingTime()
{
    if (!IsInReadyCheck())
    {
        return;
    }

    FTimespan TimeSinceStart = FDateTime::Now() - CurrentReadyCheckInfo.StartTime;
    CurrentReadyCheckInfo.RemainingTime = CurrentReadyCheckInfo.TimeoutSeconds - TimeSinceStart.GetTotalSeconds();
}

void UDBAReadyCheckSubsystem::CheckReadyCheckCompletion()
{
    if (!IsInReadyCheck())
    {
        return;
    }

    if (CurrentReadyCheckInfo.AreAllPlayersReady())
    {
        SetReadyCheckState(EDBAReadyCheckSubsystemState::AllReady);
        OnReadyCheckCompleted.Broadcast(true);

        UWorld* World = GetWorld();
        if (World)
        {
            World->GetTimerManager().ClearTimer(RemainingTimeTimerHandle);
            World->GetTimerManager().ClearTimer(TimeoutTimerHandle);
        }

        UE_LOG(LogDBAUI, Log, TEXT("[UDBAReadyCheckSubsystem] ReadyCheck 完成，所有玩家已准备"));
    }
}

void UDBAReadyCheckSubsystem::HandleReadyCheckTimeout()
{
    if (!IsInReadyCheck())
    {
        return;
    }

    UE_LOG(LogDBAUI, Warning, TEXT("[UDBAReadyCheckSubsystem] ReadyCheck 超时"));

    SetReadyCheckState(EDBAReadyCheckSubsystemState::Timeout);
    OnReadyCheckCompleted.Broadcast(false);

    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(RemainingTimeTimerHandle);
    }

    CurrentReadyCheckInfo = FDBAReadyCheckSubsystemInfo();
}

FString UDBAReadyCheckSubsystem::GetLocalPlayerId() const
{
	// 通过 PlayerController 获取本地玩家 ID
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			if (APlayerState* PlayerState = PC->GetPlayerState<APlayerState>())
			{
				return FString::FromInt(PlayerState->GetPlayerId());
			}
		}
	}

	// 降级方案：通过 PlayerController 获取唯一 ID
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			return FString::Printf(TEXT("Player_%lld"), PC->GetUniqueID());
		}
	}

	// 最终降级方案
	return TEXT("Player_Unknown");
}