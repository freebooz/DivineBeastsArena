// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Common/Session/DBAMatchFoundInfo.h"
#include "Common/Types/DBACommonEnums.h"
#include "DBAMatchEntryCoordinator.generated.h"

class UDBAQueueSubsystem;
class UDBAReadyCheckSubsystem;
class UDBAHeroSelectSubsystem;
class UDBAElementSelectSubsystem;
class UDBAFiveCampSelectSubsystem;
class UDBALoadingPreparationSubsystem;

UENUM(BlueprintType)
enum class EDBAMatchEntryPhase : uint8
{
    None UMETA(DisplayName = "无"),
    Queue UMETA(DisplayName = "排队中"),
    MatchFound UMETA(DisplayName = "匹配成功"),
    ReadyCheck UMETA(DisplayName = "准备确认"),
    HeroSelect UMETA(DisplayName = "英雄选择"),
    ElementSelect UMETA(DisplayName = "元素选择"),
    FiveCampSelect UMETA(DisplayName = "阵营选择"),
    Loading UMETA(DisplayName = "加载中"),
    MatchStart UMETA(DisplayName = "比赛开始")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAOnMatchEntryPhaseChanged, EDBAMatchEntryPhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAOnMatchEntryFailed, const FString&, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDBAOnMatchEntryCompleted);

UCLASS()
class DIVINEBEASTSARENA_API UDBAMatchEntryCoordinator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UDBAMatchEntryCoordinator();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "DBA|MatchEntry")
    void StartMatchEntry(const FString& MatchId);

    UFUNCTION(BlueprintCallable, Category = "DBA|MatchEntry")
    void CancelMatchEntry();

    UFUNCTION(BlueprintCallable, Category = "DBA|MatchEntry")
    void OnMatchFound(const FDBAMatchFoundInfo& MatchInfo);

    UFUNCTION(BlueprintCallable, Category = "DBA|MatchEntry")
    void OnReadyCheckCompleted(bool bSuccess);

    UFUNCTION(BlueprintCallable, Category = "DBA|MatchEntry")
    void OnHeroSelectCompleted(FName HeroId);

    UFUNCTION(BlueprintCallable, Category = "DBA|MatchEntry")
    void OnElementSelectCompleted(EDBAElement Element);

    UFUNCTION(BlueprintCallable, Category = "DBA|MatchEntry")
    void OnFiveCampSelectCompleted(EDBAFiveCamp FiveCamp);

    UFUNCTION(BlueprintCallable, Category = "DBA|MatchEntry")
    void OnLoadingCompleted();

    UFUNCTION(BlueprintPure, Category = "DBA|MatchEntry")
    EDBAMatchEntryPhase GetCurrentPhase() const { return CurrentPhase; }

    UFUNCTION(BlueprintPure, Category = "DBA|MatchEntry")
    const FString& GetCurrentMatchId() const { return CurrentMatchId; }

    UFUNCTION(BlueprintPure, Category = "DBA|MatchEntry")
    const FDBAMatchFoundInfo& GetMatchInfo() const { return CurrentMatchInfo; }

public:
    UPROPERTY(BlueprintAssignable, Category = "DBA|MatchEntry")
    FDBAOnMatchEntryPhaseChanged OnMatchEntryPhaseChanged;

    UPROPERTY(BlueprintAssignable, Category = "DBA|MatchEntry")
    FDBAOnMatchEntryFailed OnMatchEntryFailed;

    UPROPERTY(BlueprintAssignable, Category = "DBA|MatchEntry")
    FDBAOnMatchEntryCompleted OnMatchEntryCompleted;

private:
    void SetPhase(EDBAMatchEntryPhase NewPhase);
    void TransitionToNextPhase();
    void InitializeSubsystems();

private:
    UPROPERTY()
    EDBAMatchEntryPhase CurrentPhase = EDBAMatchEntryPhase::None;

    UPROPERTY()
    FString CurrentMatchId;

    UPROPERTY()
    FDBAMatchFoundInfo CurrentMatchInfo;

    UPROPERTY()
    TObjectPtr<UDBAQueueSubsystem> QueueSubsystem;

    UPROPERTY()
    TObjectPtr<UDBAReadyCheckSubsystem> ReadyCheckSubsystem;

    UPROPERTY()
    TObjectPtr<UDBAHeroSelectSubsystem> HeroSelectSubsystem;

    UPROPERTY()
    TObjectPtr<UDBAElementSelectSubsystem> ElementSelectSubsystem;

    UPROPERTY()
    TObjectPtr<UDBAFiveCampSelectSubsystem> FiveCampSelectSubsystem;

    UPROPERTY()
    TObjectPtr<UDBALoadingPreparationSubsystem> LoadingSubsystem;
};
