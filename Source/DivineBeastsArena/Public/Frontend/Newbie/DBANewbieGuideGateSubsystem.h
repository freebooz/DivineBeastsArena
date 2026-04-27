// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DBANewbieGuideGateSubsystem.generated.h"

/**
 * 新手引导步骤枚举
 * 定义新手引导的关键检查点
 */
UENUM(BlueprintType)
enum class EDBANewbieGuideStep : uint8
{
    None UMETA(DisplayName = "未开始"),

    CharacterCreated UMETA(DisplayName = "完成角色创建"),

    MovementTutorial UMETA(DisplayName = "完成移动教学"),

    SkillTutorial UMETA(DisplayName = "完成技能教学"),

    FirstPractice UMETA(DisplayName = "完成第一场 Practice"),

    Completed UMETA(DisplayName = "完成所有新手引导")
};

/**
 * 新手引导进度结构体
 * 存储新手引导的完成状态
 */
USTRUCT(BlueprintType)
struct FDBANewbieGuideProgress
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Newbie Guide")
    EDBANewbieGuideStep CurrentStep = EDBANewbieGuideStep::None;

    UPROPERTY(BlueprintReadOnly, Category = "Newbie Guide")
    bool bCharacterCreated = false;

    UPROPERTY(BlueprintReadOnly, Category = "Newbie Guide")
    bool bMovementTutorialCompleted = false;

    UPROPERTY(BlueprintReadOnly, Category = "Newbie Guide")
    bool bSkillTutorialCompleted = false;

    UPROPERTY(BlueprintReadOnly, Category = "Newbie Guide")
    bool bFirstPracticeCompleted = false;

    UPROPERTY(BlueprintReadOnly, Category = "Newbie Guide")
    bool bAllCompleted = false;

    bool IsFullyCompleted() const
    {
        return bCharacterCreated &&
               bMovementTutorialCompleted &&
               bSkillTutorialCompleted &&
               bFirstPracticeCompleted;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAOnNewbieGuideStepCompleted, EDBANewbieGuideStep, CompletedStep);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDBAOnNewbieGuideFullyCompleted);

UCLASS()
class DIVINEBEASTSARENA_API UDBANewbieGuideGateSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UDBANewbieGuideGateSubsystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "DBA|Newbie Guide")
    void LoadNewbieGuideProgress(const FString& CharacterId);

    UFUNCTION(BlueprintCallable, Category = "DBA|Newbie Guide")
    void MarkStepCompleted(EDBANewbieGuideStep Step);

    UFUNCTION(BlueprintPure, Category = "DBA|Newbie Guide")
    bool HasCompletedNewbieGuide() const { return CurrentProgress.bAllCompleted; }

    UFUNCTION(BlueprintPure, Category = "DBA|Newbie Guide")
    FDBANewbieGuideProgress GetCurrentProgress() const { return CurrentProgress; }

    UFUNCTION(BlueprintPure, Category = "DBA|Newbie Guide")
    bool ShouldEnterNewbieVillage() const { return !HasCompletedNewbieGuide(); }

    UFUNCTION(BlueprintCallable, Category = "DBA|Newbie Guide")
    void ResetNewbieGuideProgress();

public:
    UPROPERTY(BlueprintAssignable, Category = "DBA|Newbie Guide")
    FDBAOnNewbieGuideStepCompleted OnNewbieGuideStepCompleted;

    UPROPERTY(BlueprintAssignable, Category = "DBA|Newbie Guide")
    FDBAOnNewbieGuideFullyCompleted OnNewbieGuideFullyCompleted;

private:
    void SaveNewbieGuideProgress();
    void UpdateCompletionStatus();

private:
    UPROPERTY()
    FDBANewbieGuideProgress CurrentProgress;

    UPROPERTY()
    FString CurrentCharacterId;
};