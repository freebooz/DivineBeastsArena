// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAUserWidgetBase.h"
#include "UDBAReadyCheckWidgetBase.generated.h"

/**
 * DBAReadyCheckWidgetBase
 *
 * 准备确认 Widget 基类
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAReadyCheckWidgetBase : public UDBAUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBAReadyCheckWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|ReadyCheck")
	void ShowReadyCheck(const FText& ModeName, const FText& MapName, float TimeoutSeconds);

	UFUNCTION(BlueprintCallable, Category = "DBA|ReadyCheck")
	void AcceptReadyCheck();

	UFUNCTION(BlueprintCallable, Category = "DBA|ReadyCheck")
	void DeclineReadyCheck();

	UFUNCTION(BlueprintCallable, Category = "DBA|ReadyCheck")
	void UpdateRemainingTime(float RemainingTime);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|ReadyCheck", meta = (DisplayName = "On Ready Check Shown"))
	void BP_OnReadyCheckShown(const FText& ModeName, const FText& MapName, float TimeoutSeconds);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|ReadyCheck", meta = (DisplayName = "On Time Updated"))
	void BP_OnTimeUpdated(float RemainingTime, float Percentage);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|ReadyCheck", meta = (DisplayName = "On Ready Check Completed"))
	void BP_OnReadyCheckCompleted(bool bSuccess);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBA|ReadyCheck")
	FText CachedModeName;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|ReadyCheck")
	FText CachedMapName;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|ReadyCheck")
	float TotalTimeout;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|ReadyCheck")
	float CachedRemainingTime;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|ReadyCheck")
	bool bHasAccepted;
};
