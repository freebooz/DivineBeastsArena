// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAUserWidgetBase.h"
#include "DBAArenaHUDRootWidgetBase.generated.h"

class UDBAPlayerUnitFrameWidgetBase;
class UDBAAbilityBarWidgetBase;
class UDBAPassiveAndResonancePanelWidgetBase;
class UDBABuffBarWidgetBase;
class UDBADebuffBarWidgetBase;
class UDBACCBarWidgetBase;
class UDBASelfCastBarWidgetBase;
class UDBAMomentumPanelWidgetBase;
class UDBAChainUltimatePanelWidgetBase;
class UDBACombatAnnouncementWidgetBase;
class UDBACriticalStateHintWidgetBase;
class UDBAAuraSummaryPanelWidgetBase;
class UDBAUltimateReadyPromptWidgetBase;
class UDBAConnectionWarningWidgetBase;
class UDBAArenaObjectiveTrackerWidgetBase;
class UDBAArenaHUDWidgetController;

UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAArenaHUDRootWidgetBase : public UDBAUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBAArenaHUDRootWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeOnActivated();
	virtual void NativeOnDeactivated();

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ArenaHUD")
	void SetWidgetController(UDBAArenaHUDWidgetController* InController);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ArenaHUD")
	void SetHUDVisible(bool bVisible);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ArenaHUD")
	void SetHUDEditMode(bool bEditMode);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ArenaHUD")
	void ApplyFiveCampTheme(uint8 FiveCamp);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|ArenaHUD", meta = (DisplayName = "On Apply FiveCamp Theme"))
	void BP_OnApplyFiveCampTheme(uint8 FiveCamp);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD", meta = (BindWidget))
	TObjectPtr<UDBAPlayerUnitFrameWidgetBase> PlayerUnitFrame;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD", meta = (BindWidget))
	TObjectPtr<UDBAAbilityBarWidgetBase> AbilityBar;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD", meta = (BindWidget))
	TObjectPtr<UDBAPassiveAndResonancePanelWidgetBase> PassiveAndResonancePanel;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD", meta = (BindWidget))
	TObjectPtr<UDBABuffBarWidgetBase> BuffBar;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD", meta = (BindWidget))
	TObjectPtr<UDBADebuffBarWidgetBase> DebuffBar;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD", meta = (BindWidget))
	TObjectPtr<UDBACCBarWidgetBase> CCBar;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD", meta = (BindWidget))
	TObjectPtr<UDBASelfCastBarWidgetBase> SelfCastBar;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD", meta = (BindWidget))
	TObjectPtr<UDBAMomentumPanelWidgetBase> MomentumPanel;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD", meta = (BindWidget))
	TObjectPtr<UDBAChainUltimatePanelWidgetBase> ChainUltimatePanel;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD", meta = (BindWidget))
	TObjectPtr<UDBACombatAnnouncementWidgetBase> CombatAnnouncement;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD", meta = (BindWidget))
	TObjectPtr<UDBACriticalStateHintWidgetBase> CriticalStateHint;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD", meta = (BindWidget))
	TObjectPtr<UDBAUltimateReadyPromptWidgetBase> UltimateReadyPrompt;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD", meta = (BindWidget))
	TObjectPtr<UDBAConnectionWarningWidgetBase> ConnectionWarning;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD", meta = (BindWidget))
	TObjectPtr<UDBAArenaObjectiveTrackerWidgetBase> ObjectiveTracker;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD")
	TObjectPtr<UDBAArenaHUDWidgetController> WidgetController;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD")
	bool bIsEditMode;
};
