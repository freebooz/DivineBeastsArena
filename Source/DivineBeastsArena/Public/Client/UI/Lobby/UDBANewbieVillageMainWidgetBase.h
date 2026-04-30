// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAUserWidgetBase.h"
#include "DBANewbieVillageMainWidgetBase.generated.h"

class UDBANewbieTaskTrackerWidgetBase;

/**
 * DBANewbieVillageMainWidgetBase
 *
 * 新手村主界面 Widget 基类
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBANewbieVillageMainWidgetBase : public UDBAUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBANewbieVillageMainWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|NewbieVillage")
	void RefreshTaskTracker();

	UFUNCTION(BlueprintCallable, Category = "DBA|NewbieVillage")
	void ShowGatePrompt(bool bIsLocked, const FText& UnlockCondition);

	UFUNCTION(BlueprintCallable, Category = "DBA|NewbieVillage")
	void SkipNewbieVillage();

	UFUNCTION(BlueprintCallable, Category = "DBA|NewbieVillage")
	void EnterMainLobby();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|NewbieVillage", meta = (DisplayName = "On Task Tracker Refreshed"))
	void BP_OnTaskTrackerRefreshed();

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|NewbieVillage", meta = (DisplayName = "On Gate Prompt Shown"))
	void BP_OnGatePromptShown(bool bIsLocked, const FText& UnlockCondition);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBA|NewbieVillage", meta = (BindWidget))
	TObjectPtr<UDBANewbieTaskTrackerWidgetBase> TaskTracker;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|NewbieVillage")
	bool bIsMainLobbyUnlocked;
};
