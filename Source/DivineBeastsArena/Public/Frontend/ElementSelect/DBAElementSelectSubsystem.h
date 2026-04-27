// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DBAElementSelectSubsystem.generated.h"

UENUM(BlueprintType)
enum class EDBAElementSelectState : uint8
{
    None UMETA(DisplayName = "未开始"),
    Selecting UMETA(DisplayName = "选择中"),
    Selected UMETA(DisplayName = "已选择")
};

USTRUCT(BlueprintType)
struct FDBAElementSelectInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "ElementSelect")
    FString SelectSessionId;

    UPROPERTY(BlueprintReadOnly, Category = "ElementSelect")
    EDBAElementSelectState State = EDBAElementSelectState::None;

    UPROPERTY(BlueprintReadOnly, Category = "ElementSelect")
    EDBAElement SelectedElement = EDBAElement::None;

    UPROPERTY(BlueprintReadOnly, Category = "ElementSelect")
    EDBAElement SuggestedElement = EDBAElement::None;

    UPROPERTY(BlueprintReadOnly, Category = "ElementSelect")
    FName SelectedHeroId;

    UPROPERTY(BlueprintReadOnly, Category = "ElementSelect")
    float RemainingTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "ElementSelect")
    float TotalTime = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAOnElementSelectStateChanged, EDBAElementSelectState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAOnElementSelected, EDBAElement, Element);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDBAOnElementSelectTimeUpdate, float, RemainingTime, float, TotalTime);

UCLASS()
class DIVINEBEASTSARENA_API UDBAElementSelectSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UDBAElementSelectSubsystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "DBA|ElementSelect")
    void StartElementSelect(FName HeroId, float SelectTimeSeconds);

    UFUNCTION(BlueprintCallable, Category = "DBA|ElementSelect")
    void SelectElement(EDBAElement Element);

    UFUNCTION(BlueprintCallable, Category = "DBA|ElementSelect")
    void ConfirmElementSelection();

    UFUNCTION(BlueprintCallable, Category = "DBA|ElementSelect")
    void CancelSelection();

    UFUNCTION(BlueprintPure, Category = "DBA|ElementSelect")
    bool IsInElementSelect() const { return CurrentSelectInfo.State == EDBAElementSelectState::Selecting; }

    UFUNCTION(BlueprintPure, Category = "DBA|ElementSelect")
    FDBAElementSelectInfo GetCurrentSelectInfo() const { return CurrentSelectInfo; }

    UFUNCTION(BlueprintPure, Category = "DBA|ElementSelect")
    EDBAElement GetSelectedElement() const { return CurrentSelectInfo.SelectedElement; }

    UFUNCTION(BlueprintCallable, Category = "DBA|ElementSelect")
    void SetSuggestedElement(EDBAElement Element);

public:
    UPROPERTY(BlueprintAssignable, Category = "DBA|ElementSelect")
    FDBAOnElementSelectStateChanged OnElementSelectStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "DBA|ElementSelect")
    FDBAOnElementSelected OnElementSelected;

    UPROPERTY(BlueprintAssignable, Category = "DBA|ElementSelect")
    FDBAOnElementSelectTimeUpdate OnElementSelectTimeUpdate;

private:
    void SetSelectState(EDBAElementSelectState NewState);
    void UpdateRemainingTime();
    void HandleSelectTimeout();

private:
    UPROPERTY()
    FDBAElementSelectInfo CurrentSelectInfo;

    FTimerHandle RemainingTimeTimerHandle;
    FTimerHandle TimeoutTimerHandle;

    static constexpr float TimeUpdateFrequency = 0.1f;
};
