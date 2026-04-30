// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAUserWidgetBase.h"
#include "UDBAQueueModeSelectWidgetBase.generated.h"

UENUM(BlueprintType)
enum class EDBAQueueModeSelectMode : uint8
{
	QuickMatch UMETA(DisplayName = "快速匹配"),
	Ranked UMETA(DisplayName = "排位赛"),
	Custom UMETA(DisplayName = "自定义"),
	Practice UMETA(DisplayName = "练习模式")
};

USTRUCT(BlueprintType)
struct FDBAQueueModeSelectData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "DBA|QueueMode")
	EDBAQueueModeSelectMode Mode;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|QueueMode")
	FText ModeName;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|QueueMode")
	FText ModeDescription;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|QueueMode")
	FText MapName;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|QueueMode")
	FText EstimatedWaitTime;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|QueueMode")
	bool bIsAvailable;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|QueueMode")
	FText UnavailableReason;

	FDBAQueueModeSelectData()
		: Mode(EDBAQueueModeSelectMode::QuickMatch)
		, bIsAvailable(true)
	{
	}
};

/**
 * DBAQueueModeSelectWidgetBase
 *
 * 匹配模式选择 Widget 基类
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAQueueModeSelectWidgetBase : public UDBAUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBAQueueModeSelectWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|QueueModeSelect")
	void RefreshModeList();

	UFUNCTION(BlueprintCallable, Category = "DBA|QueueModeSelect")
	void SelectMode(EDBAQueueModeSelectMode Mode);

	UFUNCTION(BlueprintCallable, Category = "DBA|QueueModeSelect")
	void StartQueue();

	UFUNCTION(BlueprintCallable, Category = "DBA|QueueModeSelect")
	void CancelSelect();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|QueueModeSelect", meta = (DisplayName = "On Mode List Refreshed"))
	void BP_OnModeListRefreshed(const TArray<FDBAQueueModeSelectData>& Modes);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|QueueModeSelect", meta = (DisplayName = "On Mode Selection Changed"))
	void BP_OnModeSelectionChanged(EDBAQueueModeSelectMode NewMode);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBA|QueueModeSelect")
	TArray<FDBAQueueModeSelectData> ModeList;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|QueueModeSelect")
	EDBAQueueModeSelectMode SelectedMode;
};
