// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DBAQueueSubsystem.generated.h"

class UDBAPartySubsystem;

UENUM(BlueprintType)
enum class EDBAQueueSubsystemMode : uint8
{
    QuickMatch UMETA(DisplayName = "快速匹配"),
    Ranked UMETA(DisplayName = "排位赛"),
    Custom UMETA(DisplayName = "自定义房间")
};

UENUM(BlueprintType)
enum class EDBAQueueSubsystemState : uint8
{
    NotInQueue UMETA(DisplayName = "未在队列中"),
    Queueing UMETA(DisplayName = "正在排队"),
    MatchFound UMETA(DisplayName = "找到匹配"),
    Cancelled UMETA(DisplayName = "队列已取消"),
    Timeout UMETA(DisplayName = "队列超时")
};

USTRUCT(BlueprintType)
struct FDBAQueueSubsystemInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Queue")
    FString QueueId;

    UPROPERTY(BlueprintReadOnly, Category = "Queue")
    EDBAQueueSubsystemMode QueueMode = EDBAQueueSubsystemMode::QuickMatch;

    UPROPERTY(BlueprintReadOnly, Category = "Queue")
    EDBAQueueSubsystemState QueueState = EDBAQueueSubsystemState::NotInQueue;

    UPROPERTY(BlueprintReadOnly, Category = "Queue")
    FDateTime QueueStartTime;

    UPROPERTY(BlueprintReadOnly, Category = "Queue")
    float EstimatedWaitTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Queue")
    float CurrentWaitTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Queue")
    bool bIsPartyQueue = false;

    UPROPERTY(BlueprintReadOnly, Category = "Queue")
    int32 PartyMemberCount = 1;
};

USTRUCT(BlueprintType)
struct FDBAQueueSubsystemMatchInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Match")
    FString MatchId;

    UPROPERTY(BlueprintReadOnly, Category = "Match")
    FString MapName;

    UPROPERTY(BlueprintReadOnly, Category = "Match")
    FString GameMode;

    UPROPERTY(BlueprintReadOnly, Category = "Match")
    int32 AverageLevel = 1;

    UPROPERTY(BlueprintReadOnly, Category = "Match")
    float ReadyCheckTimeout = 30.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAOnQueueSubsystemStateChanged, EDBAQueueSubsystemState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAOnQueueSubsystemMatchFound, const FDBAQueueSubsystemMatchInfo&, MatchInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAOnQueueSubsystemCancelled, const FString&, Reason);

UCLASS()
class DIVINEBEASTSARENA_API UDBAQueueSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UDBAQueueSubsystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "DBA|Queue")
    void JoinQueue(EDBAQueueSubsystemMode QueueMode);

    UFUNCTION(BlueprintCallable, Category = "DBA|Queue")
    void LeaveQueue();

    UFUNCTION(BlueprintPure, Category = "DBA|Queue")
    bool IsInQueue() const { return CurrentQueueInfo.QueueState == EDBAQueueSubsystemState::Queueing; }

    UFUNCTION(BlueprintPure, Category = "DBA|Queue")
    FDBAQueueSubsystemInfo GetCurrentQueueInfo() const { return CurrentQueueInfo; }

    UFUNCTION(BlueprintPure, Category = "DBA|Queue")
    EDBAQueueSubsystemState GetQueueState() const { return CurrentQueueInfo.QueueState; }

public:
    UPROPERTY(BlueprintAssignable, Category = "DBA|Queue")
    FDBAOnQueueSubsystemStateChanged OnQueueStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "DBA|Queue")
    FDBAOnQueueSubsystemMatchFound OnMatchFound;

    UPROPERTY(BlueprintAssignable, Category = "DBA|Queue")
    FDBAOnQueueSubsystemCancelled OnQueueCancelled;

private:
    void SetQueueState(EDBAQueueSubsystemState NewState);
    void UpdateQueueWaitTime();
    void CheckQueueTimeout();
    void SimulateMatchFound();
    bool ValidateQueueConditions(FString& OutReason) const;

private:
    UPROPERTY()
    FDBAQueueSubsystemInfo CurrentQueueInfo;

    UPROPERTY()
    TObjectPtr<UDBAPartySubsystem> PartySubsystem;

    FTimerHandle QueueUpdateTimerHandle;
    FTimerHandle QueueTimeoutTimerHandle;
    FTimerHandle MockMatchTimerHandle;

    static constexpr float MaxQueueWaitTime = 300.0f;
    static constexpr float QueueUpdateFrequency = 1.0f;
    static constexpr float MockMatchDelay = 5.0f;
};
