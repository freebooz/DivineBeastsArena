// Copyright FreeboozStudio. All Rights Reserved.

#include "Frontend/Queue/DBAQueueSubsystem.h"
#include "Frontend/Party/DBAPartySubsystem.h"
#include "Frontend/Newbie/DBANewbieGuideGateSubsystem.h"
#include "Common/DBALogChannels.h"
#include "TimerManager.h"

UDBAQueueSubsystem::UDBAQueueSubsystem()
{
}

void UDBAQueueSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    PartySubsystem = GetGameInstance()->GetSubsystem<UDBAPartySubsystem>();

    UE_LOG(LogDBAUI, Log, TEXT("[UDBAQueueSubsystem] 初始化完成"));
}

void UDBAQueueSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(QueueUpdateTimerHandle);
        World->GetTimerManager().ClearTimer(QueueTimeoutTimerHandle);
        World->GetTimerManager().ClearTimer(MockMatchTimerHandle);
    }

    Super::Deinitialize();
}

void UDBAQueueSubsystem::JoinQueue(EDBAQueueSubsystemMode QueueMode)
{
    if (IsInQueue())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[UDBAQueueSubsystem] 已在队列中"));
        return;
    }

    FString Reason;
    if (!ValidateQueueConditions(Reason))
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[UDBAQueueSubsystem] 无法加入队列: %s"), *Reason);
        OnQueueCancelled.Broadcast(Reason);
        return;
    }

    CurrentQueueInfo.QueueId = FGuid::NewGuid().ToString();
    CurrentQueueInfo.QueueMode = QueueMode;
    CurrentQueueInfo.QueueStartTime = FDateTime::Now();
    CurrentQueueInfo.EstimatedWaitTime = 60.0f;
    CurrentQueueInfo.CurrentWaitTime = 0.0f;

    if (PartySubsystem && PartySubsystem->IsInParty())
    {
        CurrentQueueInfo.bIsPartyQueue = true;
        CurrentQueueInfo.PartyMemberCount = PartySubsystem->GetCurrentPartyInfo().Members.Num();
    }
    else
    {
        CurrentQueueInfo.bIsPartyQueue = false;
        CurrentQueueInfo.PartyMemberCount = 1;
    }

    SetQueueState(EDBAQueueSubsystemState::Queueing);

    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().SetTimer(
            QueueUpdateTimerHandle,
            this,
            &UDBAQueueSubsystem::UpdateQueueWaitTime,
            QueueUpdateFrequency,
            true
        );

        World->GetTimerManager().SetTimer(
            QueueTimeoutTimerHandle,
            this,
            &UDBAQueueSubsystem::CheckQueueTimeout,
            MaxQueueWaitTime,
            false
        );

        World->GetTimerManager().SetTimer(
            MockMatchTimerHandle,
            this,
            &UDBAQueueSubsystem::SimulateMatchFound,
            MockMatchDelay,
            false
        );
    }

    UE_LOG(LogDBAUI, Log, TEXT("[UDBAQueueSubsystem] 加入队列: 模式=%d, Party=%s"),
        static_cast<int32>(QueueMode),
        CurrentQueueInfo.bIsPartyQueue ? TEXT("是") : TEXT("否"));
}

void UDBAQueueSubsystem::LeaveQueue()
{
    if (!IsInQueue())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[UDBAQueueSubsystem] 当前不在队列中"));
        return;
    }

    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(QueueUpdateTimerHandle);
        World->GetTimerManager().ClearTimer(QueueTimeoutTimerHandle);
        World->GetTimerManager().ClearTimer(MockMatchTimerHandle);
    }

    SetQueueState(EDBAQueueSubsystemState::Cancelled);
    OnQueueCancelled.Broadcast(TEXT("玩家主动取消"));

    UE_LOG(LogDBAUI, Log, TEXT("[UDBAQueueSubsystem] 离开队列"));

    CurrentQueueInfo = FDBAQueueSubsystemInfo();
}

void UDBAQueueSubsystem::SetQueueState(EDBAQueueSubsystemState NewState)
{
    if (CurrentQueueInfo.QueueState != NewState)
    {
        CurrentQueueInfo.QueueState = NewState;
        OnQueueStateChanged.Broadcast(NewState);
        UE_LOG(LogDBAUI, Log, TEXT("[UDBAQueueSubsystem] 队列状态变更: %d"), static_cast<int32>(NewState));
    }
}

void UDBAQueueSubsystem::UpdateQueueWaitTime()
{
    if (!IsInQueue())
    {
        return;
    }

    FTimespan TimeSinceStart = FDateTime::Now() - CurrentQueueInfo.QueueStartTime;
    CurrentQueueInfo.CurrentWaitTime = TimeSinceStart.GetTotalSeconds();
}

void UDBAQueueSubsystem::CheckQueueTimeout()
{
    if (!IsInQueue())
    {
        return;
    }

    UE_LOG(LogDBAUI, Warning, TEXT("[UDBAQueueSubsystem] 队列超时"));

    SetQueueState(EDBAQueueSubsystemState::Timeout);
    OnQueueCancelled.Broadcast(TEXT("队列超时"));

    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(QueueUpdateTimerHandle);
        World->GetTimerManager().ClearTimer(MockMatchTimerHandle);
    }

    CurrentQueueInfo = FDBAQueueSubsystemInfo();
}

void UDBAQueueSubsystem::SimulateMatchFound()
{
    if (!IsInQueue())
    {
        return;
    }

    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(QueueUpdateTimerHandle);
        World->GetTimerManager().ClearTimer(QueueTimeoutTimerHandle);
        World->GetTimerManager().ClearTimer(MockMatchTimerHandle);
    }

    SetQueueState(EDBAQueueSubsystemState::MatchFound);

    FDBAQueueSubsystemMatchInfo MatchInfo;
    MatchInfo.MatchId = FGuid::NewGuid().ToString();
    MatchInfo.MapName = TEXT("Arena_Default");
    MatchInfo.GameMode = TEXT("5v5");
    MatchInfo.AverageLevel = 10;
    MatchInfo.ReadyCheckTimeout = 30.0f;

    OnMatchFound.Broadcast(MatchInfo);

    UE_LOG(LogDBAUI, Log, TEXT("[UDBAQueueSubsystem] 找到匹配: %s"), *MatchInfo.MatchId);
}

bool UDBAQueueSubsystem::ValidateQueueConditions(FString& OutReason) const
{
    UDBANewbieGuideGateSubsystem* NewbieGuideGateSubsystem = GetGameInstance()->GetSubsystem<UDBANewbieGuideGateSubsystem>();
    if (NewbieGuideGateSubsystem && !NewbieGuideGateSubsystem->HasCompletedNewbieGuide())
    {
        OutReason = TEXT("需要完成新手引导");
        return false;
    }

    if (PartySubsystem && PartySubsystem->IsInParty())
    {
        if (!PartySubsystem->IsPartyLeader())
        {
            OutReason = TEXT("只有队长可以加入队列");
            return false;
        }

        FDBAPartySubsystemInfo PartyInfo = PartySubsystem->GetCurrentPartyInfo();
        if (!PartyInfo.AreAllMembersReady())
        {
            OutReason = TEXT("Party 成员未全部准备");
            return false;
        }
    }

    OutReason.Empty();
    return true;
}