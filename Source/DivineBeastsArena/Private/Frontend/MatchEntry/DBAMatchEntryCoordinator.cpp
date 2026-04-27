// Copyright FreeboozStudio. All Rights Reserved.

#include "Frontend/MatchEntry/DBAMatchEntryCoordinator.h"
#include "Frontend/Queue/DBAQueueSubsystem.h"
#include "Common/DBALogChannels.h"
#include "Frontend/ReadyCheck/DBAReadyCheckSubsystem.h"
#include "Frontend/HeroSelect/DBAHeroSelectSubsystem.h"
#include "Frontend/ElementSelect/DBAElementSelectSubsystem.h"
#include "Frontend/FiveCampSelect/DBAFiveCampSelectSubsystem.h"
#include "Frontend/LoadingPreparation/DBALoadingPreparationSubsystem.h"

UDBAMatchEntryCoordinator::UDBAMatchEntryCoordinator()
{
}

void UDBAMatchEntryCoordinator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    InitializeSubsystems();

    UE_LOG(LogDBAUI, Log, TEXT("[UDBAMatchEntryCoordinator] 初始化完成"));
}

void UDBAMatchEntryCoordinator::Deinitialize()
{
    Super::Deinitialize();
}

void UDBAMatchEntryCoordinator::InitializeSubsystems()
{
    QueueSubsystem = GetGameInstance()->GetSubsystem<UDBAQueueSubsystem>();
    ReadyCheckSubsystem = GetGameInstance()->GetSubsystem<UDBAReadyCheckSubsystem>();
    HeroSelectSubsystem = GetGameInstance()->GetSubsystem<UDBAHeroSelectSubsystem>();
    ElementSelectSubsystem = GetGameInstance()->GetSubsystem<UDBAElementSelectSubsystem>();
    FiveCampSelectSubsystem = GetGameInstance()->GetSubsystem<UDBAFiveCampSelectSubsystem>();
    LoadingSubsystem = GetGameInstance()->GetSubsystem<UDBALoadingPreparationSubsystem>();
}

void UDBAMatchEntryCoordinator::StartMatchEntry(const FString& MatchId)
{
    if (CurrentPhase != EDBAMatchEntryPhase::None)
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[UDBAMatchEntryCoordinator] 已经在匹配流程中"));
        return;
    }

    CurrentMatchId = MatchId;
    SetPhase(EDBAMatchEntryPhase::Queue);

    UE_LOG(LogDBAUI, Log, TEXT("[UDBAMatchEntryCoordinator] 开始匹配流程: MatchId=%s"), *MatchId);
}

void UDBAMatchEntryCoordinator::CancelMatchEntry()
{
    UE_LOG(LogDBAUI, Log, TEXT("[UDBAMatchEntryCoordinator] 取消匹配流程"));

    if (QueueSubsystem && QueueSubsystem->IsInQueue())
    {
        QueueSubsystem->LeaveQueue();
    }

    SetPhase(EDBAMatchEntryPhase::None);
    CurrentMatchId.Empty();
    CurrentMatchInfo = FDBAMatchFoundInfo();
}

void UDBAMatchEntryCoordinator::OnMatchFound(const FDBAMatchFoundInfo& MatchInfo)
{
    if (CurrentPhase != EDBAMatchEntryPhase::Queue)
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[UDBAMatchEntryCoordinator] 当前不在排队阶段"));
        return;
    }

    CurrentMatchInfo = MatchInfo;
    SetPhase(EDBAMatchEntryPhase::MatchFound);

    UE_LOG(LogDBAUI, Log, TEXT("[UDBAMatchEntryCoordinator] 匹配成功: MatchId=%s"), *MatchInfo.MatchId);
}

void UDBAMatchEntryCoordinator::OnReadyCheckCompleted(bool bSuccess)
{
    if (CurrentPhase != EDBAMatchEntryPhase::ReadyCheck)
    {
        return;
    }

    if (bSuccess)
    {
        SetPhase(EDBAMatchEntryPhase::HeroSelect);

        if (HeroSelectSubsystem)
        {
            HeroSelectSubsystem->StartHeroSelect(30.0f);
        }
    }
    else
    {
        OnMatchEntryFailed.Broadcast(TEXT("准备确认失败"));
        SetPhase(EDBAMatchEntryPhase::None);
    }
}

void UDBAMatchEntryCoordinator::OnHeroSelectCompleted(FName HeroId)
{
    if (CurrentPhase != EDBAMatchEntryPhase::HeroSelect)
    {
        return;
    }

    SetPhase(EDBAMatchEntryPhase::ElementSelect);

    if (ElementSelectSubsystem)
    {
        ElementSelectSubsystem->StartElementSelect(HeroId, 20.0f);
    }

    UE_LOG(LogDBAUI, Log, TEXT("[UDBAMatchEntryCoordinator] 英雄选择完成: HeroId=%s"), *HeroId.ToString());
}

void UDBAMatchEntryCoordinator::OnElementSelectCompleted(EDBAElement Element)
{
    if (CurrentPhase != EDBAMatchEntryPhase::ElementSelect)
    {
        return;
    }

    SetPhase(EDBAMatchEntryPhase::FiveCampSelect);

    if (FiveCampSelectSubsystem)
    {
        FName HeroId = HeroSelectSubsystem ? HeroSelectSubsystem->GetSelectedHeroId() : NAME_None;
        FiveCampSelectSubsystem->StartFiveCampSelect(HeroId, Element, 15.0f);
    }

    UE_LOG(LogDBAUI, Log, TEXT("[UDBAMatchEntryCoordinator] 元素选择完成: Element=%d"), static_cast<int32>(Element));
}

void UDBAMatchEntryCoordinator::OnFiveCampSelectCompleted(EDBAFiveCamp FiveCamp)
{
    if (CurrentPhase != EDBAMatchEntryPhase::FiveCampSelect)
    {
        return;
    }

    SetPhase(EDBAMatchEntryPhase::Loading);

    if (LoadingSubsystem)
    {
        FDBA_MATCHLoadInfo MatchLoadInfo;
        MatchLoadInfo.MatchId = CurrentMatchInfo.MatchId;
        MatchLoadInfo.MapName = CurrentMatchInfo.MapName;
        MatchLoadInfo.GameMode = CurrentMatchInfo.GameMode;
        LoadingSubsystem->InitializeMatchLoadInfo(MatchLoadInfo);
    }

    UE_LOG(LogDBAUI, Log, TEXT("[UDBAMatchEntryCoordinator] 阵营选择完成: FiveCamp=%d"), static_cast<int32>(FiveCamp));
}

void UDBAMatchEntryCoordinator::OnLoadingCompleted()
{
    if (CurrentPhase != EDBAMatchEntryPhase::Loading)
    {
        return;
    }

    SetPhase(EDBAMatchEntryPhase::MatchStart);
    OnMatchEntryCompleted.Broadcast();

    UE_LOG(LogDBAUI, Log, TEXT("[UDBAMatchEntryCoordinator] 加载完成，比赛开始"));

    SetPhase(EDBAMatchEntryPhase::None);
}

void UDBAMatchEntryCoordinator::SetPhase(EDBAMatchEntryPhase NewPhase)
{
    if (CurrentPhase != NewPhase)
    {
        CurrentPhase = NewPhase;
        OnMatchEntryPhaseChanged.Broadcast(NewPhase);
        UE_LOG(LogDBAUI, Log, TEXT("[UDBAMatchEntryCoordinator] 匹配流程阶段变更: %d"), static_cast<int32>(NewPhase));
    }
}

void UDBAMatchEntryCoordinator::TransitionToNextPhase()
{
}
