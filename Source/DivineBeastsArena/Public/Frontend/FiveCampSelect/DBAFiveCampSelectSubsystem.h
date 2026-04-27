// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Common/Types/DBACommonEnums.h"
#include "DBAFiveCampSelectSubsystem.generated.h"

UENUM(BlueprintType)
enum class EDBAFiveCampSelectState : uint8
{
    None UMETA(DisplayName = "未开始"),
    Selecting UMETA(DisplayName = "选择中"),
    Selected UMETA(DisplayName = "已选择")
};

USTRUCT(BlueprintType)
struct FDBAFiveCampSelectInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "FiveCampSelect")
    FString SelectSessionId;

    UPROPERTY(BlueprintReadOnly, Category = "FiveCampSelect")
    EDBAFiveCampSelectState State = EDBAFiveCampSelectState::None;

    UPROPERTY(BlueprintReadOnly, Category = "FiveCampSelect")
    EDBAFiveCamp SelectedFiveCamp = EDBAFiveCamp::None;

    UPROPERTY(BlueprintReadOnly, Category = "FiveCampSelect")
    FName SelectedHeroId;

    UPROPERTY(BlueprintReadOnly, Category = "FiveCampSelect")
    EDBAElement SelectedElement;

    UPROPERTY(BlueprintReadOnly, Category = "FiveCampSelect")
    float RemainingTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "FiveCampSelect")
    float TotalTime = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAOnFiveCampSelectStateChanged, EDBAFiveCampSelectState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAOnFiveCampSelected, EDBAFiveCamp, FiveCamp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDBAOnFiveCampSelectTimeUpdate, float, RemainingTime, float, TotalTime);

UCLASS()
class DIVINEBEASTSARENA_API UDBAFiveCampSelectSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UDBAFiveCampSelectSubsystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "DBA|FiveCampSelect")
    void StartFiveCampSelect(FName HeroId, EDBAElement Element, float SelectTimeSeconds);

    UFUNCTION(BlueprintCallable, Category = "DBA|FiveCampSelect")
    void SelectFiveCamp(EDBAFiveCamp FiveCamp);

    UFUNCTION(BlueprintCallable, Category = "DBA|FiveCampSelect")
    void ConfirmFiveCampSelection();

    UFUNCTION(BlueprintCallable, Category = "DBA|FiveCampSelect")
    void CancelSelection();

    UFUNCTION(BlueprintPure, Category = "DBA|FiveCampSelect")
    bool IsInFiveCampSelect() const { return CurrentSelectInfo.State == EDBAFiveCampSelectState::Selecting; }

    UFUNCTION(BlueprintPure, Category = "DBA|FiveCampSelect")
    FDBAFiveCampSelectInfo GetCurrentSelectInfo() const { return CurrentSelectInfo; }

    UFUNCTION(BlueprintPure, Category = "DBA|FiveCampSelect")
    EDBAFiveCamp GetSelectedFiveCamp() const { return CurrentSelectInfo.SelectedFiveCamp; }

public:
    UPROPERTY(BlueprintAssignable, Category = "DBA|FiveCampSelect")
    FDBAOnFiveCampSelectStateChanged OnFiveCampSelectStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "DBA|FiveCampSelect")
    FDBAOnFiveCampSelected OnFiveCampSelected;

    UPROPERTY(BlueprintAssignable, Category = "DBA|FiveCampSelect")
    FDBAOnFiveCampSelectTimeUpdate OnFiveCampSelectTimeUpdate;

private:
    void SetSelectState(EDBAFiveCampSelectState NewState);
    void UpdateRemainingTime();
    void HandleSelectTimeout();

private:
    UPROPERTY()
    FDBAFiveCampSelectInfo CurrentSelectInfo;

    FTimerHandle RemainingTimeTimerHandle;
    FTimerHandle TimeoutTimerHandle;

    static constexpr float TimeUpdateFrequency = 0.1f;
};
