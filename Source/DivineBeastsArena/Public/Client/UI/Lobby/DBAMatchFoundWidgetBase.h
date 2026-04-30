// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAUserWidgetBase.h"
#include "DBAMatchFoundWidgetBase.generated.h"

/**
 * DBAMatchFoundWidgetBase
 *
 * 匹配成功 Widget 基类
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAMatchFoundWidgetBase : public UDBAUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBAMatchFoundWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|MatchFound")
	void ShowMatchFound(const FText& ModeName, const FText& MapName);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|MatchFound", meta = (DisplayName = "On Match Found Shown"))
	void BP_OnMatchFoundShown(const FText& ModeName, const FText& MapName);

	void AutoNavigateToReadyCheck();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBA|MatchFound")
	FText CachedModeName;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|MatchFound")
	FText CachedMapName;

	FTimerHandle AutoNavigateTimerHandle;
};
