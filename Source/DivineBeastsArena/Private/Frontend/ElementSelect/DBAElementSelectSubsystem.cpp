// Copyright FreeboozStudio. All Rights Reserved.

#include "Frontend/ElementSelect/DBAElementSelectSubsystem.h"
#include "Common/DBALogChannels.h"
#include "TimerManager.h"

UDBAElementSelectSubsystem::UDBAElementSelectSubsystem()
{
}

void UDBAElementSelectSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LogDBAUI, Log, TEXT("[UDBAElementSelectSubsystem] 初始化完成"));
}

void UDBAElementSelectSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(RemainingTimeTimerHandle);
        World->GetTimerManager().ClearTimer(TimeoutTimerHandle);
    }

    Super::Deinitialize();
}

void UDBAElementSelectSubsystem::StartElementSelect(FName HeroId, float SelectTimeSeconds)
{
    if (IsInElementSelect())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[UDBAElementSelectSubsystem] 已在元素选择中"));
        return;
    }

    CurrentSelectInfo.SelectSessionId = FGuid::NewGuid().ToString();
    CurrentSelectInfo.State = EDBAElementSelectState::Selecting;
    CurrentSelectInfo.SelectedHeroId = HeroId;
    CurrentSelectInfo.SelectedElement = EDBAElement::None;
    CurrentSelectInfo.RemainingTime = SelectTimeSeconds;
    CurrentSelectInfo.TotalTime = SelectTimeSeconds;

    SetSelectState(EDBAElementSelectState::Selecting);

    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().SetTimer(
            RemainingTimeTimerHandle,
            this,
            &UDBAElementSelectSubsystem::UpdateRemainingTime,
            TimeUpdateFrequency,
            true
        );

        World->GetTimerManager().SetTimer(
            TimeoutTimerHandle,
            this,
            &UDBAElementSelectSubsystem::HandleSelectTimeout,
            SelectTimeSeconds,
            false
        );
    }

    UE_LOG(LogDBAUI, Log, TEXT("[UDBAElementSelectSubsystem] 开始元素选择: HeroId=%s, Time=%.1f"), *HeroId.ToString(), SelectTimeSeconds);
}

void UDBAElementSelectSubsystem::SelectElement(EDBAElement Element)
{
    if (!IsInElementSelect())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[UDBAElementSelectSubsystem] 当前不在元素选择中"));
        return;
    }

    CurrentSelectInfo.SelectedElement = Element;
    OnElementSelected.Broadcast(Element);

    UE_LOG(LogDBAUI, Log, TEXT("[UDBAElementSelectSubsystem] 选择元素: %d"), static_cast<int32>(Element));
}

void UDBAElementSelectSubsystem::ConfirmElementSelection()
{
    if (!IsInElementSelect())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[UDBAElementSelectSubsystem] 当前不在元素选择中"));
        return;
    }

    if (CurrentSelectInfo.SelectedElement == EDBAElement::None)
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[UDBAElementSelectSubsystem] 未选择元素"));
        return;
    }

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(RemainingTimeTimerHandle);
        World->GetTimerManager().ClearTimer(TimeoutTimerHandle);
    }

    SetSelectState(EDBAElementSelectState::Selected);
    OnElementSelected.Broadcast(CurrentSelectInfo.SelectedElement);

    UE_LOG(LogDBAUI, Log, TEXT("[UDBAElementSelectSubsystem] 确认元素选择"));
}

void UDBAElementSelectSubsystem::CancelSelection()
{
    if (!IsInElementSelect())
    {
        return;
    }

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(RemainingTimeTimerHandle);
        World->GetTimerManager().ClearTimer(TimeoutTimerHandle);
    }

    SetSelectState(EDBAElementSelectState::None);
    CurrentSelectInfo = FDBAElementSelectInfo();

    UE_LOG(LogDBAUI, Log, TEXT("[UDBAElementSelectSubsystem] 取消元素选择"));
}

void UDBAElementSelectSubsystem::SetSelectState(EDBAElementSelectState NewState)
{
    if (CurrentSelectInfo.State != NewState)
    {
        CurrentSelectInfo.State = NewState;
        OnElementSelectStateChanged.Broadcast(NewState);
        UE_LOG(LogDBAUI, Log, TEXT("[UDBAElementSelectSubsystem] 元素选择状态变更: %d"), static_cast<int32>(NewState));
    }
}

void UDBAElementSelectSubsystem::UpdateRemainingTime()
{
    if (!IsInElementSelect())
    {
        return;
    }

    CurrentSelectInfo.RemainingTime = FMath::Max(0.0f, CurrentSelectInfo.RemainingTime - TimeUpdateFrequency);
    OnElementSelectTimeUpdate.Broadcast(CurrentSelectInfo.RemainingTime, CurrentSelectInfo.TotalTime);
}

void UDBAElementSelectSubsystem::HandleSelectTimeout()
{
    if (!IsInElementSelect())
    {
        return;
    }

    UE_LOG(LogDBAUI, Warning, TEXT("[UDBAElementSelectSubsystem] 元素选择超时"));

    if (CurrentSelectInfo.SelectedElement == EDBAElement::None && CurrentSelectInfo.SuggestedElement != EDBAElement::None)
    {
        SelectElement(CurrentSelectInfo.SuggestedElement);
    }

    ConfirmElementSelection();
}

void UDBAElementSelectSubsystem::SetSuggestedElement(EDBAElement Element)
{
    CurrentSelectInfo.SuggestedElement = Element;
}
