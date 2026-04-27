// Copyright FreeboozStudio. All Rights Reserved.
// 元素选择子系统实现 - 管理英雄元素选择的逻辑

#include "Frontend/ElementSelect/DBAElementSelectSubsystem.h"
#include "Common/DBALogChannels.h"
#include "TimerManager.h"

// 构造函数 - 初始化元素选择子系统
UDBAElementSelectSubsystem::UDBAElementSelectSubsystem()
{
}

// 初始化子系统 - 设置日志记录
void UDBAElementSelectSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LogDBAUI, Log, TEXT("[UDBAElementSelectSubsystem] 初始化完成"));
}

// 反初始化子系统 - 清理所有定时器
void UDBAElementSelectSubsystem::Deinitialize()
{
    // 清理剩余时间更新定时器
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(RemainingTimeTimerHandle);
        World->GetTimerManager().ClearTimer(TimeoutTimerHandle);
    }

    Super::Deinitialize();
}

// 开始元素选择 - 启动指定英雄的元素选择流程
void UDBAElementSelectSubsystem::StartElementSelect(FName HeroId, float SelectTimeSeconds)
{
    // 检查是否已在元素选择中
    if (IsInElementSelect())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[UDBAElementSelectSubsystem] 已在元素选择中"));
        return;
    }

    // 初始化选择会话信息
    CurrentSelectInfo.SelectSessionId = FGuid::NewGuid().ToString();
    CurrentSelectInfo.State = EDBAElementSelectState::Selecting;
    CurrentSelectInfo.SelectedHeroId = HeroId;
    CurrentSelectInfo.SelectedElement = EDBAElement::None;
    CurrentSelectInfo.RemainingTime = SelectTimeSeconds;
    CurrentSelectInfo.TotalTime = SelectTimeSeconds;

    // 更新选择状态
    SetSelectState(EDBAElementSelectState::Selecting);

    // 获取世界上下文用于设置定时器
    UWorld* World = GetWorld();
    if (World)
    {
        // 设置剩余时间更新定时器（高频更新UI）
        World->GetTimerManager().SetTimer(
            RemainingTimeTimerHandle,
            this,
            &UDBAElementSelectSubsystem::UpdateRemainingTime,
            TimeUpdateFrequency,
            true
        );

        // 设置选择超时定时器
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

// 选择元素 - 玩家选择指定元素
void UDBAElementSelectSubsystem::SelectElement(EDBAElement Element)
{
    // 检查是否在元素选择中
    if (!IsInElementSelect())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[UDBAElementSelectSubsystem] 当前不在元素选择中"));
        return;
    }

    // 更新当前选择的元素
    CurrentSelectInfo.SelectedElement = Element;

    // 广播元素选择事件
    OnElementSelected.Broadcast(Element);

    UE_LOG(LogDBAUI, Log, TEXT("[UDBAElementSelectSubsystem] 选择元素: %d"), static_cast<int32>(Element));
}

// 确认元素选择 - 玩家确认当前选择，完成选择流程
void UDBAElementSelectSubsystem::ConfirmElementSelection()
{
    // 检查是否在元素选择中
    if (!IsInElementSelect())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[UDBAElementSelectSubsystem] 当前不在元素选择中"));
        return;
    }

    // 检查是否已选择元素
    if (CurrentSelectInfo.SelectedElement == EDBAElement::None)
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[UDBAElementSelectSubsystem] 未选择元素"));
        return;
    }

    // 停止所有定时器
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(RemainingTimeTimerHandle);
        World->GetTimerManager().ClearTimer(TimeoutTimerHandle);
    }

    // 更新状态为已选择并广播事件
    SetSelectState(EDBAElementSelectState::Selected);
    OnElementSelected.Broadcast(CurrentSelectInfo.SelectedElement);

    UE_LOG(LogDBAUI, Log, TEXT("[UDBAElementSelectSubsystem] 确认元素选择"));
}

// 取消选择 - 玩家取消当前元素选择
void UDBAElementSelectSubsystem::CancelSelection()
{
    // 如果不在选择中则直接返回
    if (!IsInElementSelect())
    {
        return;
    }

    // 停止所有定时器
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(RemainingTimeTimerHandle);
        World->GetTimerManager().ClearTimer(TimeoutTimerHandle);
    }

    // 重置选择状态和会话信息
    SetSelectState(EDBAElementSelectState::None);
    CurrentSelectInfo = FDBAElementSelectInfo();

    UE_LOG(LogDBAUI, Log, TEXT("[UDBAElementSelectSubsystem] 取消元素选择"));
}

// 设置选择状态 - 内部方法，更新选择状态并广播事件
void UDBAElementSelectSubsystem::SetSelectState(EDBAElementSelectState NewState)
{
    if (CurrentSelectInfo.State != NewState)
    {
        CurrentSelectInfo.State = NewState;
        OnElementSelectStateChanged.Broadcast(NewState);
        UE_LOG(LogDBAUI, Log, TEXT("[UDBAElementSelectSubsystem] 元素选择状态变更: %d"), static_cast<int32>(NewState));
    }
}

// 更新剩余时间 - 定时器回调，减少剩余时间并广播更新事件
void UDBAElementSelectSubsystem::UpdateRemainingTime()
{
    // 如果不在选择中则直接返回
    if (!IsInElementSelect())
    {
        return;
    }

    // 减少剩余时间（确保不会小于0）
    CurrentSelectInfo.RemainingTime = FMath::Max(0.0f, CurrentSelectInfo.RemainingTime - TimeUpdateFrequency);

    // 广播时间更新事件
    OnElementSelectTimeUpdate.Broadcast(CurrentSelectInfo.RemainingTime, CurrentSelectInfo.TotalTime);
}

// 处理选择超时 - 超时定时器回调，自动选择建议元素或确认
void UDBAElementSelectSubsystem::HandleSelectTimeout()
{
    // 如果不在选择中则直接返回
    if (!IsInElementSelect())
    {
        return;
    }

    UE_LOG(LogDBAUI, Warning, TEXT("[UDBAElementSelectSubsystem] 元素选择超时"));

    // 如果玩家未选择但有建议元素，则自动选择建议元素
    if (CurrentSelectInfo.SelectedElement == EDBAElement::None && CurrentSelectInfo.SuggestedElement != EDBAElement::None)
    {
        SelectElement(CurrentSelectInfo.SuggestedElement);
    }

    // 确认当前选择（无论是否已选择元素）
    ConfirmElementSelection();
}

// 设置建议元素 - 设置推荐选择的元素（用于AI或系统推荐）
void UDBAElementSelectSubsystem::SetSuggestedElement(EDBAElement Element)
{
    CurrentSelectInfo.SuggestedElement = Element;
}