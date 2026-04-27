// Copyright FreeboozStudio. All Rights Reserved.

#include "Frontend/HeroSelect/DBAHeroSelectSubsystem.h"
#include "Data/DBAZodiacHeroData.h"
#include "Common/DBALogChannels.h"
#include "TimerManager.h"

UDBAHeroSelectSubsystem::UDBAHeroSelectSubsystem()
{
}

void UDBAHeroSelectSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LogDBAUI, Log, TEXT("[UDBAHeroSelectSubsystem] 初始化完成"));
}

void UDBAHeroSelectSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(RemainingTimeTimerHandle);
        World->GetTimerManager().ClearTimer(TimeoutTimerHandle);
    }

    Super::Deinitialize();
}

void UDBAHeroSelectSubsystem::StartHeroSelect(float SelectTimeSeconds)
{
    if (IsInHeroSelect())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[UDBAHeroSelectSubsystem] 已在英雄选择中"));
        return;
    }

    CurrentSelectInfo.SelectSessionId = FGuid::NewGuid().ToString();
    CurrentSelectInfo.State = EDBAHeroSelectState::Selecting;
    CurrentSelectInfo.RemainingTime = SelectTimeSeconds;
    CurrentSelectInfo.TotalTime = SelectTimeSeconds;
    CurrentSelectInfo.SelectedHeroId = NAME_None;
    CurrentSelectInfo.bHasConfirmed = false;

    SetSelectState(EDBAHeroSelectState::Selecting);

    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().SetTimer(
            RemainingTimeTimerHandle,
            this,
            &UDBAHeroSelectSubsystem::UpdateRemainingTime,
            TimeUpdateFrequency,
            true
        );

        World->GetTimerManager().SetTimer(
            TimeoutTimerHandle,
            this,
            &UDBAHeroSelectSubsystem::HandleSelectTimeout,
            SelectTimeSeconds,
            false
        );
    }

    UE_LOG(LogDBAUI, Log, TEXT("[UDBAHeroSelectSubsystem] 开始英雄选择: Time=%.1f"), SelectTimeSeconds);
}

void UDBAHeroSelectSubsystem::SelectHero(FName HeroId)
{
    if (!IsInHeroSelect())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[UDBAHeroSelectSubsystem] 当前不在英雄选择中"));
        return;
    }

    if (CurrentSelectInfo.bHasConfirmed)
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[UDBAHeroSelectSubsystem] 已确认，无法更换英雄"));
        return;
    }

    CurrentSelectInfo.SelectedHeroId = HeroId;
    OnHeroSelected.Broadcast(HeroId);

    UE_LOG(LogDBAUI, Log, TEXT("[UDBAHeroSelectSubsystem] 选择英雄: %s"), *HeroId.ToString());
}

void UDBAHeroSelectSubsystem::ConfirmHeroSelection()
{
    if (!IsInHeroSelect())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[UDBAHeroSelectSubsystem] 当前不在英雄选择中"));
        return;
    }

    if (CurrentSelectInfo.SelectedHeroId.IsNone())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[UDBAHeroSelectSubsystem] 未选择英雄"));
        return;
    }

    if (CurrentSelectInfo.bHasConfirmed)
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[UDBAHeroSelectSubsystem] 已确认"));
        return;
    }

    CurrentSelectInfo.bHasConfirmed = true;
    OnHeroConfirmChanged.Broadcast(true);

    SetSelectState(EDBAHeroSelectState::Confirmed);

    UE_LOG(LogDBAUI, Log, TEXT("[UDBAHeroSelectSubsystem] 确认英雄选择"));
}

void UDBAHeroSelectSubsystem::CancelSelection()
{
    if (!IsInHeroSelect())
    {
        return;
    }

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(RemainingTimeTimerHandle);
        World->GetTimerManager().ClearTimer(TimeoutTimerHandle);
    }

    SetSelectState(EDBAHeroSelectState::None);
    CurrentSelectInfo = FDBAHeroSelectInfo();

    UE_LOG(LogDBAUI, Log, TEXT("[UDBAHeroSelectSubsystem] 取消英雄选择"));
}

void UDBAHeroSelectSubsystem::SetSelectState(EDBAHeroSelectState NewState)
{
    if (CurrentSelectInfo.State != NewState)
    {
        CurrentSelectInfo.State = NewState;
        OnHeroSelectStateChanged.Broadcast(NewState);
        UE_LOG(LogDBAUI, Log, TEXT("[UDBAHeroSelectSubsystem] 英雄选择状态变更: %d"), static_cast<int32>(NewState));
    }
}

void UDBAHeroSelectSubsystem::UpdateRemainingTime()
{
    if (!IsInHeroSelect())
    {
        return;
    }

    CurrentSelectInfo.RemainingTime = FMath::Max(0.0f, CurrentSelectInfo.RemainingTime - TimeUpdateFrequency);
    OnHeroSelectTimeUpdate.Broadcast(CurrentSelectInfo.RemainingTime, CurrentSelectInfo.TotalTime);
}

void UDBAHeroSelectSubsystem::HandleSelectTimeout()
{
    if (!IsInHeroSelect())
    {
        return;
    }

    UE_LOG(LogDBAUI, Warning, TEXT("[UDBAHeroSelectSubsystem] 英雄选择超时"));

    if (!CurrentSelectInfo.bHasConfirmed && !CurrentSelectInfo.SelectedHeroId.IsNone())
    {
        ConfirmHeroSelection();
    }

    SetSelectState(EDBAHeroSelectState::LockedIn);
}

void UDBAHeroSelectSubsystem::SetAvailableHeroIds(const TArray<FName>& HeroIds)
{
    CurrentSelectInfo.AvailableHeroIds = HeroIds;
}
