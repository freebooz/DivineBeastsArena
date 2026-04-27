// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Common/Types/DBACommonEnums.h"
#include "DBALoadingPreparationSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FDBAPlayerLoadInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Loading")
    FString PlayerId;

    UPROPERTY(BlueprintReadOnly, Category = "Loading")
    FString PlayerName;

    UPROPERTY(BlueprintReadOnly, Category = "Loading")
    FName HeroId;

    UPROPERTY(BlueprintReadOnly, Category = "Loading")
    EDBAZodiac Zodiac;

    UPROPERTY(BlueprintReadOnly, Category = "Loading")
    EDBAElement Element;

    UPROPERTY(BlueprintReadOnly, Category = "Loading")
    EDBAFiveCamp FiveCamp;

    UPROPERTY(BlueprintReadOnly, Category = "Loading")
    FName PassiveSkillId;

    UPROPERTY(BlueprintReadOnly, Category = "Loading")
    FName Skill01Id;

    UPROPERTY(BlueprintReadOnly, Category = "Loading")
    FName Skill02Id;

    UPROPERTY(BlueprintReadOnly, Category = "Loading")
    FName Skill03Id;

    UPROPERTY(BlueprintReadOnly, Category = "Loading")
    FName Skill04Id;

    UPROPERTY(BlueprintReadOnly, Category = "Loading")
    FName UltimateSkillId;

    UPROPERTY(BlueprintReadOnly, Category = "Loading")
    int32 ResonanceLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Loading")
    bool bIsLoadComplete = false;

    FDBAPlayerLoadInfo()
        : Zodiac(EDBAZodiac::None)
        , Element(EDBAElement::None)
        , FiveCamp(EDBAFiveCamp::None)
        , ResonanceLevel(0)
        , bIsLoadComplete(false)
    {
    }
};

USTRUCT(BlueprintType)
struct FDBA_MATCHLoadInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Loading")
    FString MatchId;

    UPROPERTY(BlueprintReadOnly, Category = "Loading")
    FString MapName;

    UPROPERTY(BlueprintReadOnly, Category = "Loading")
    FString GameMode;

    UPROPERTY(BlueprintReadOnly, Category = "Loading")
    TArray<FDBAPlayerLoadInfo> BlueTeamPlayers;

    UPROPERTY(BlueprintReadOnly, Category = "Loading")
    TArray<FDBAPlayerLoadInfo> RedTeamPlayers;

    UPROPERTY(BlueprintReadOnly, Category = "Loading")
    float TotalLoadingTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Loading")
    int32 TotalPlayerCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Loading")
    int32 LoadedPlayerCount = 0;

    FDBA_MATCHLoadInfo()
        : TotalLoadingTime(0.0f)
        , TotalPlayerCount(0)
        , LoadedPlayerCount(0)
    {
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAOnMatchLoadInfoReady, const FDBA_MATCHLoadInfo&, MatchInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAOnPlayerLoadProgress, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAOnAllPlayersLoaded, bool, bSuccess);

UCLASS()
class DIVINEBEASTSARENA_API UDBALoadingPreparationSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UDBALoadingPreparationSubsystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "DBA|Loading")
    void InitializeMatchLoadInfo(const FDBA_MATCHLoadInfo& MatchInfo);

    UFUNCTION(BlueprintCallable, Category = "DBA|Loading")
    void UpdatePlayerLoadProgress(const FString& PlayerId, bool bLoadComplete);

    UFUNCTION(BlueprintCallable, Category = "DBA|Loading")
    float GetOverallLoadProgress() const;

    UFUNCTION(BlueprintPure, Category = "DBA|Loading")
    const FDBA_MATCHLoadInfo& GetMatchLoadInfo() const { return CurrentMatchInfo; }

    UFUNCTION(BlueprintPure, Category = "DBA|Loading")
    bool AreAllPlayersLoaded() const;

    UFUNCTION(BlueprintCallable, Category = "DBA|Loading")
    void StartPostLoading();

    UFUNCTION(BlueprintCallable, Category = "DBA|Loading")
    void CancelLoading();

public:
    UPROPERTY(BlueprintAssignable, Category = "DBA|Loading")
    FDBAOnMatchLoadInfoReady OnMatchLoadInfoReady;

    UPROPERTY(BlueprintAssignable, Category = "DBA|Loading")
    FDBAOnPlayerLoadProgress OnPlayerLoadProgress;

    UPROPERTY(BlueprintAssignable, Category = "DBA|Loading")
    FDBAOnAllPlayersLoaded OnAllPlayersLoaded;

private:
    UPROPERTY()
    FDBA_MATCHLoadInfo CurrentMatchInfo;

    bool bIsLoading = false;
};
