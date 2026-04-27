// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DBAHeroSelectSubsystem.generated.h"

class UDBAHeroDataAsset;

UENUM(BlueprintType)
enum class EDBAHeroSelectState : uint8
{
    None UMETA(DisplayName = "未开始"),
    Selecting UMETA(DisplayName = "选择中"),
    Confirmed UMETA(DisplayName = "已确认"),
    LockedIn UMETA(DisplayName = "已锁定")
};

USTRUCT(BlueprintType)
struct FDBAHeroSelectInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "HeroSelect")
    FString SelectSessionId;

    UPROPERTY(BlueprintReadOnly, Category = "HeroSelect")
    EDBAHeroSelectState State = EDBAHeroSelectState::None;

    UPROPERTY(BlueprintReadOnly, Category = "HeroSelect")
    TArray<FName> AvailableHeroIds;

    UPROPERTY(BlueprintReadOnly, Category = "HeroSelect")
    FName SelectedHeroId;

    UPROPERTY(BlueprintReadOnly, Category = "HeroSelect")
    bool bHasConfirmed = false;

    UPROPERTY(BlueprintReadOnly, Category = "HeroSelect")
    float RemainingTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "HeroSelect")
    float TotalTime = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAOnHeroSelectStateChanged, EDBAHeroSelectState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAOnHeroSelected, FName, HeroId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAOnHeroConfirmChanged, bool, bConfirmed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDBAOnHeroSelectTimeUpdate, float, RemainingTime, float, TotalTime);

UCLASS()
class DIVINEBEASTSARENA_API UDBAHeroSelectSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UDBAHeroSelectSubsystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "DBA|HeroSelect")
    void StartHeroSelect(float SelectTimeSeconds);

    UFUNCTION(BlueprintCallable, Category = "DBA|HeroSelect")
    void SelectHero(FName HeroId);

    UFUNCTION(BlueprintCallable, Category = "DBA|HeroSelect")
    void ConfirmHeroSelection();

    UFUNCTION(BlueprintCallable, Category = "DBA|HeroSelect")
    void CancelSelection();

    UFUNCTION(BlueprintPure, Category = "DBA|HeroSelect")
    bool IsInHeroSelect() const { return CurrentSelectInfo.State == EDBAHeroSelectState::Selecting; }

    UFUNCTION(BlueprintPure, Category = "DBA|HeroSelect")
    FDBAHeroSelectInfo GetCurrentSelectInfo() const { return CurrentSelectInfo; }

    UFUNCTION(BlueprintPure, Category = "DBA|HeroSelect")
    FName GetSelectedHeroId() const { return CurrentSelectInfo.SelectedHeroId; }

    UFUNCTION(BlueprintPure, Category = "DBA|HeroSelect")
    bool HasConfirmed() const { return CurrentSelectInfo.bHasConfirmed; }

    UFUNCTION(BlueprintPure, Category = "DBA|HeroSelect")
    TArray<FName> GetAvailableHeroIds() const { return CurrentSelectInfo.AvailableHeroIds; }

    UFUNCTION(BlueprintCallable, Category = "DBA|HeroSelect")
    void SetAvailableHeroIds(const TArray<FName>& HeroIds);

public:
    UPROPERTY(BlueprintAssignable, Category = "DBA|HeroSelect")
    FDBAOnHeroSelectStateChanged OnHeroSelectStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "DBA|HeroSelect")
    FDBAOnHeroSelected OnHeroSelected;

    UPROPERTY(BlueprintAssignable, Category = "DBA|HeroSelect")
    FDBAOnHeroConfirmChanged OnHeroConfirmChanged;

    UPROPERTY(BlueprintAssignable, Category = "DBA|HeroSelect")
    FDBAOnHeroSelectTimeUpdate OnHeroSelectTimeUpdate;

private:
    void SetSelectState(EDBAHeroSelectState NewState);
    void UpdateRemainingTime();
    void HandleSelectTimeout();

private:
    UPROPERTY()
    FDBAHeroSelectInfo CurrentSelectInfo;

    FTimerHandle RemainingTimeTimerHandle;
    FTimerHandle TimeoutTimerHandle;

    static constexpr float TimeUpdateFrequency = 0.1f;
};
