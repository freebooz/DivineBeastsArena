// Copyright FreeboozStudio. All Rights Reserved.

#include "Frontend/FiveCampSelect/DBAFiveCampSelectSubsystem.h"
#include "Common/DBALogChannels.h"
#include "TimerManager.h"

UDBAFiveCampSelectSubsystem::UDBAFiveCampSelectSubsystem()
{
}

void UDBAFiveCampSelectSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LogDBAUI, Log, TEXT("[UDBAFiveCampSelectSubsystem] 初始化完成"));
}

void UDBAFiveCampSelectSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(RemainingTimeTimerHandle);
        World->GetTimerManager().ClearTimer(TimeoutTimerHandle);
    }

    Super::Deinitialize();
}

void UDBAFiveCampSelectSubsystem::StartFiveCampSelect(FName HeroId, EDBAElement Element, float SelectTimeSeconds)
{
    if (IsInFiveCampSelect())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[UDBAFiveCampSelectSubsystem] 已在五大阵营选择中"));
        return;
    }

    CurrentSelectInfo.SelectSessionId = FGuid::NewGuid().ToString();
    CurrentSelectInfo.State = EDBAFiveCampSelectState::Selecting;
    CurrentSelectInfo.SelectedHeroId = HeroId;
    CurrentSelectInfo.SelectedElement = Element;
    CurrentSelectInfo.SelectedFiveCamp = EDBAFiveCamp::None;
    CurrentSelectInfo.RemainingTime = SelectTimeSeconds;
    CurrentSelectInfo.TotalTime = SelectTimeSeconds;

    SetSelectState(EDBAFiveCampSelectState::Selecting);

    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().SetTimer(
            RemainingTimeTimerHandle,
            this,
            &UDBAFiveCampSelectSubsystem::UpdateRemainingTime,
            TimeUpdateFrequency,
            true
        );

        World->GetTimerManager().SetTimer(
            TimeoutTimerHandle,
            this,
            &UDBAFiveCampSelectSubsystem::HandleSelectTimeout,
            SelectTimeSeconds,
            false
        );
    }

    UE_LOG(LogDBAUI, Log, TEXT("[UDBAFiveCampSelectSubsystem] 开始五大阵营选择: HeroId=%s, Element=%d, Time=%.1f"),
        *HeroId.ToString(), static_cast<int32>(Element), SelectTimeSeconds);
}

void UDBAFiveCampSelectSubsystem::SelectFiveCamp(EDBAFiveCamp FiveCamp)
{
    if (!IsInFiveCampSelect())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[UDBAFiveCampSelectSubsystem] 当前不在五大阵营选择中"));
        return;
    }

    CurrentSelectInfo.SelectedFiveCamp = FiveCamp;
    OnFiveCampSelected.Broadcast(FiveCamp);

    UE_LOG(LogDBAUI, Log, TEXT("[UDBAFiveCampSelectSubsystem] 选择五大阵营: %d"), static_cast<int32>(FiveCamp));
}

void UDBAFiveCampSelectSubsystem::ConfirmFiveCampSelection()
{
    if (!IsInFiveCampSelect())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[UDBAFiveCampSelectSubsystem] 当前不在五大阵营选择中"));
        return;
    }

    if (CurrentSelectInfo.SelectedFiveCamp == EDBAFiveCamp::None)
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[UDBAFiveCampSelectSubsystem] 未选择五大阵营"));
        return;
    }

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(RemainingTimeTimerHandle);
        World->GetTimerManager().ClearTimer(TimeoutTimerHandle);
    }

    SetSelectState(EDBAFiveCampSelectState::Selected);
    OnFiveCampSelected.Broadcast(CurrentSelectInfo.SelectedFiveCamp);

    UE_LOG(LogDBAUI, Log, TEXT("[UDBAFiveCampSelectSubsystem] 确认五大阵营选择"));
}

void UDBAFiveCampSelectSubsystem::CancelSelection()
{
    if (!IsInFiveCampSelect())
    {
        return;
    }

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(RemainingTimeTimerHandle);
        World->GetTimerManager().ClearTimer(TimeoutTimerHandle);
    }

    SetSelectState(EDBAFiveCampSelectState::None);
    CurrentSelectInfo = FDBAFiveCampSelectInfo();

    UE_LOG(LogDBAUI, Log, TEXT("[UDBAFiveCampSelectSubsystem] 取消五大阵营选择"));
}

void UDBAFiveCampSelectSubsystem::SetSelectState(EDBAFiveCampSelectState NewState)
{
    if (CurrentSelectInfo.State != NewState)
    {
        CurrentSelectInfo.State = NewState;
        OnFiveCampSelectStateChanged.Broadcast(NewState);
        UE_LOG(LogDBAUI, Log, TEXT("[UDBAFiveCampSelectSubsystem] 五大阵营选择状态变更: %d"), static_cast<int32>(NewState));
    }
}

void UDBAFiveCampSelectSubsystem::UpdateRemainingTime()
{
    if (!IsInFiveCampSelect())
    {
        return;
    }

    CurrentSelectInfo.RemainingTime = FMath::Max(0.0f, CurrentSelectInfo.RemainingTime - TimeUpdateFrequency);
    OnFiveCampSelectTimeUpdate.Broadcast(CurrentSelectInfo.RemainingTime, CurrentSelectInfo.TotalTime);
}

void UDBAFiveCampSelectSubsystem::HandleSelectTimeout()
{
    if (!IsInFiveCampSelect())
    {
        return;
    }

    UE_LOG(LogDBAUI, Warning, TEXT("[UDBAFiveCampSelectSubsystem] 五大阵营选择超时"));

    if (CurrentSelectInfo.SelectedFiveCamp == EDBAFiveCamp::None)
    {
        SelectFiveCamp(EDBAFiveCamp::Center);
    }

    ConfirmFiveCampSelection();
}
