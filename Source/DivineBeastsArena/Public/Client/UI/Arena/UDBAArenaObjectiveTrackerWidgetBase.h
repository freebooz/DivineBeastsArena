// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAUserWidgetBase.h"
#include "UDBAArenaObjectiveTrackerWidgetBase.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAArenaObjectiveTrackerWidgetBase : public UDBAUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBAArenaObjectiveTrackerWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct();
	virtual void NativeDestruct();

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ObjectiveTracker")
	void UpdateObjective(const FText& ObjectiveText, float Progress);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ObjectiveTracker")
	void CompleteObjective();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|ObjectiveTracker", meta = (DisplayName = "On Objective Updated"))
	void BP_OnObjectiveUpdated(const FText& ObjectiveText, float Progress);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|ObjectiveTracker", meta = (DisplayName = "On Objective Completed"))
	void BP_OnObjectiveCompleted();
};
