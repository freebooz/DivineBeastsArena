// Copyright FreeboozStudio. All Rights Reserved.

#include "Frontend/Newbie/DBANewbieGuideGateSubsystem.h"
#include "Common/DBALogChannels.h"

UDBANewbieGuideGateSubsystem::UDBANewbieGuideGateSubsystem()
{
}

void UDBANewbieGuideGateSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LogDBAUI, Log, TEXT("[DBANewbieGuideGateSubsystem] 初始化完成"));
}

void UDBANewbieGuideGateSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

void UDBANewbieGuideGateSubsystem::LoadNewbieGuideProgress(const FString& CharacterId)
{
    if (CharacterId.IsEmpty())
    {
        UE_LOG(LogDBAUI, Warning, TEXT("[DBANewbieGuideGateSubsystem] 角色 ID 为空，无法加载新手引导进度"));
        return;
    }

    CurrentCharacterId = CharacterId;

    CurrentProgress = FDBANewbieGuideProgress();
    CurrentProgress.bCharacterCreated = true;

    UE_LOG(LogDBAUI, Log, TEXT("[DBANewbieGuideGateSubsystem] 加载新手引导进度: %s"), *CharacterId);
}

void UDBANewbieGuideGateSubsystem::MarkStepCompleted(EDBANewbieGuideStep Step)
{
    bool bStepChanged = false;

    switch (Step)
    {
    case EDBANewbieGuideStep::CharacterCreated:
        if (!CurrentProgress.bCharacterCreated)
        {
            CurrentProgress.bCharacterCreated = true;
            bStepChanged = true;
        }
        break;

    case EDBANewbieGuideStep::MovementTutorial:
        if (!CurrentProgress.bMovementTutorialCompleted)
        {
            CurrentProgress.bMovementTutorialCompleted = true;
            bStepChanged = true;
        }
        break;

    case EDBANewbieGuideStep::SkillTutorial:
        if (!CurrentProgress.bSkillTutorialCompleted)
        {
            CurrentProgress.bSkillTutorialCompleted = true;
            bStepChanged = true;
        }
        break;

    case EDBANewbieGuideStep::FirstPractice:
        if (!CurrentProgress.bFirstPracticeCompleted)
        {
            CurrentProgress.bFirstPracticeCompleted = true;
            bStepChanged = true;
        }
        break;

    default:
        break;
    }

    if (bStepChanged)
    {
        CurrentProgress.CurrentStep = Step;
        UpdateCompletionStatus();
        SaveNewbieGuideProgress();
        OnNewbieGuideStepCompleted.Broadcast(Step);

        UE_LOG(LogDBAUI, Log, TEXT("[DBANewbieGuideGateSubsystem] 完成新手引导步骤: %d"), static_cast<int32>(Step));
    }
}

void UDBANewbieGuideGateSubsystem::ResetNewbieGuideProgress()
{
    CurrentProgress = FDBANewbieGuideProgress();
    SaveNewbieGuideProgress();

    UE_LOG(LogDBAUI, Log, TEXT("[DBANewbieGuideGateSubsystem] 重置新手引导进度"));
}

void UDBANewbieGuideGateSubsystem::SaveNewbieGuideProgress()
{
    UE_LOG(LogDBAUI, Log, TEXT("[DBANewbieGuideGateSubsystem] 保存新手引导进度"));
}

void UDBANewbieGuideGateSubsystem::UpdateCompletionStatus()
{
    bool bWasCompleted = CurrentProgress.bAllCompleted;
    CurrentProgress.bAllCompleted = CurrentProgress.IsFullyCompleted();

    if (!bWasCompleted && CurrentProgress.bAllCompleted)
    {
        CurrentProgress.CurrentStep = EDBANewbieGuideStep::Completed;
        OnNewbieGuideFullyCompleted.Broadcast();
        UE_LOG(LogDBAUI, Log, TEXT("[DBANewbieGuideGateSubsystem] 新手引导全部完成"));
    }
}