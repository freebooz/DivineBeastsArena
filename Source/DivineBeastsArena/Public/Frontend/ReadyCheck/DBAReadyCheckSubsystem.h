// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Common/Queue/DBAReadyCheckTypes.h"
#include "DBAReadyCheckSubsystem.generated.h"

UENUM(BlueprintType)
enum class EDBAReadyCheckSubsystemState : uint8
{
    None UMETA(DisplayName = "未开始"),
    WaitingForPlayers UMETA(DisplayName = "等待玩家确认"),
    AllReady UMETA(DisplayName = "全部准备"),
    Declined UMETA(DisplayName = "有玩家拒绝"),
    Timeout UMETA(DisplayName = "超时")
};

UENUM(BlueprintType)
enum class EDBAReadyCheckPlayerState : uint8
{
    Pending UMETA(DisplayName = "等待确认"),
    Ready UMETA(DisplayName = "已准备"),
    Declined UMETA(DisplayName = "已拒绝")
};

USTRUCT(BlueprintType)
struct FDBAReadyCheckSubsystemPlayerInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "ReadyCheck")
    FString PlayerId;

    UPROPERTY(BlueprintReadOnly, Category = "ReadyCheck")
    FString DisplayName;

    UPROPERTY(BlueprintReadOnly, Category = "ReadyCheck")
    EDBAReadyCheckPlayerState State = EDBAReadyCheckPlayerState::Pending;

    UPROPERTY(BlueprintReadOnly, Category = "ReadyCheck")
    bool bIsLocalPlayer = false;
};

USTRUCT(BlueprintType)
struct FDBAReadyCheckSubsystemInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "ReadyCheck")
    FString ReadyCheckId;

    UPROPERTY(BlueprintReadOnly, Category = "ReadyCheck")
    FString MatchId;

    UPROPERTY(BlueprintReadOnly, Category = "ReadyCheck")
    EDBAReadyCheckSubsystemState State = EDBAReadyCheckSubsystemState::None;

    UPROPERTY(BlueprintReadOnly, Category = "ReadyCheck")
    TArray<FDBAReadyCheckSubsystemPlayerInfo> Players;

    UPROPERTY(BlueprintReadOnly, Category = "ReadyCheck")
    float TimeoutSeconds = 30.0f;

    UPROPERTY(BlueprintReadOnly, Category = "ReadyCheck")
    float RemainingTime = 30.0f;

    UPROPERTY(BlueprintReadOnly, Category = "ReadyCheck")
    FDateTime StartTime;

    int32 GetReadyPlayerCount() const
    {
        int32 Count = 0;
        for (const FDBAReadyCheckSubsystemPlayerInfo& Player : Players)
        {
            if (Player.State == EDBAReadyCheckPlayerState::Ready)
            {
                Count++;
            }
        }
        return Count;
    }

    bool AreAllPlayersReady() const
    {
        for (const FDBAReadyCheckSubsystemPlayerInfo& Player : Players)
        {
            if (Player.State != EDBAReadyCheckPlayerState::Ready)
            {
                return false;
            }
        }
        return Players.Num() > 0;
    }

    bool HasPlayerDeclined() const
    {
        for (const FDBAReadyCheckSubsystemPlayerInfo& Player : Players)
        {
            if (Player.State == EDBAReadyCheckPlayerState::Declined)
            {
                return true;
            }
        }
        return false;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAOnReadyCheckSubsystemStateChanged, EDBAReadyCheckSubsystemState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDBAOnReadyCheckPlayerStateChanged, const FString&, PlayerId, EDBAReadyCheckPlayerState, NewState);

UCLASS()
class DIVINEBEASTSARENA_API UDBAReadyCheckSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UDBAReadyCheckSubsystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "DBA|ReadyCheck")
    void StartReadyCheck(const FString& MatchId, float TimeoutSeconds);

    UFUNCTION(BlueprintCallable, Category = "DBA|ReadyCheck")
    void AcceptReadyCheck();

    UFUNCTION(BlueprintCallable, Category = "DBA|ReadyCheck")
    void DeclineReadyCheck();

    UFUNCTION(BlueprintPure, Category = "DBA|ReadyCheck")
    bool IsInReadyCheck() const { return CurrentReadyCheckInfo.State == EDBAReadyCheckSubsystemState::WaitingForPlayers; }

    UFUNCTION(BlueprintPure, Category = "DBA|ReadyCheck")
    FDBAReadyCheckSubsystemInfo GetCurrentReadyCheckInfo() const { return CurrentReadyCheckInfo; }

    UFUNCTION(BlueprintPure, Category = "DBA|ReadyCheck")
    EDBAReadyCheckSubsystemState GetReadyCheckState() const { return CurrentReadyCheckInfo.State; }

public:
    UPROPERTY(BlueprintAssignable, Category = "DBA|ReadyCheck")
    FDBAOnReadyCheckSubsystemStateChanged OnReadyCheckStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "DBA|ReadyCheck")
    FDBAOnReadyCheckPlayerStateChanged OnReadyCheckPlayerStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "DBA|ReadyCheck")
    FDBAOnReadyCheckCompletedMulticast OnReadyCheckCompleted;

private:
    void SetReadyCheckState(EDBAReadyCheckSubsystemState NewState);
    void UpdatePlayerState(const FString& PlayerId, EDBAReadyCheckPlayerState NewState);
    void UpdateRemainingTime();
    void CheckReadyCheckCompletion();
    void HandleReadyCheckTimeout();
    FString GetLocalPlayerId() const;

private:
    UPROPERTY()
    FDBAReadyCheckSubsystemInfo CurrentReadyCheckInfo;

    FTimerHandle RemainingTimeTimerHandle;
    FTimerHandle TimeoutTimerHandle;

    static constexpr float RemainingTimeUpdateFrequency = 0.1f;
};