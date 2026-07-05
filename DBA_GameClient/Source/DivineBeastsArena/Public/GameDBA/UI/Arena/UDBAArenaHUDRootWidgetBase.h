// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#pragma once

#include "CoreMinimal.h"
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "UDBAArenaHUDRootWidgetBase.generated.h"

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
class UDBAArenaEventFeedWidgetBase;
class UDBAAuraSummaryPanelWidgetBase;
class UDBAUltimateReadyPromptWidgetBase;
class UDBAConnectionWarningWidgetBase;
class UDBAArenaObjectiveTrackerWidgetBase;
class UDBAArenaHUDWidgetController;
class UDBAPlayerUnitFrameWidgetController;
class ADBAZodiacCharacterBase;

UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAArenaHUDRootWidgetBase : public UDBAMobaUserWidgetBase
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
	void SetPlayerUnitFrameWidgetController(UDBAPlayerUnitFrameWidgetController* InController);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ArenaHUD")
	void BindArenaHUDToCharacter(ADBAZodiacCharacterBase* InCharacter);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ArenaHUD")
	void SetHUDVisible(bool bVisible);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ArenaHUD")
	void SetHUDEditMode(bool bEditMode);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|ArenaHUD")
	void ApplyFiveCampTheme(uint8 FiveCamp);

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|ArenaHUD", meta = (DisplayName = "On Apply FiveCamp Theme"))
	void BP_OnApplyFiveCampTheme(uint8 FiveCamp);

	UFUNCTION()
	void HandleControllerUltimateEnergyUpdated(float CurrentEnergy, float MaxEnergy);

	UFUNCTION()
	void HandleControllerChainLevelUpdated(int32 ChainLevel);

	UFUNCTION()
	void HandleControllerResonanceLevelUpdated(int32 ResonanceLevel);

	UFUNCTION()
	void HandleControllerMomentumUpdated(int32 MomentumLevel, float MomentumProgress);

	UFUNCTION()
	void HandleControllerStatusBuffAdded(const FString& BuffId, float Duration);

	UFUNCTION()
	void HandleControllerStatusBuffRemoved(const FString& BuffId);

	UFUNCTION()
	void HandleControllerStatusBuffsCleared();

	UFUNCTION()
	void HandleControllerStatusDebuffAdded(const FString& DebuffId, float Duration);

	UFUNCTION()
	void HandleControllerStatusDebuffRemoved(const FString& DebuffId);

	UFUNCTION()
	void HandleControllerStatusDebuffsCleared();

	UFUNCTION()
	void HandleControllerStatusCCEffectAdded(const FString& CCId, float Duration);

	UFUNCTION()
	void HandleControllerStatusCCEffectRemoved(const FString& CCId);

	UFUNCTION()
	void HandleControllerStatusCCEffectsCleared();

	UFUNCTION()
	void HandleControllerCombatAnnouncementShown(const FText& Text, float Duration);

	UFUNCTION()
	void HandleControllerCombatAnnouncementCleared();

	UFUNCTION()
	void HandleControllerCriticalStateHintsChanged(bool bLowHP, bool bLowEnergy);

	UFUNCTION()
	void HandleControllerArenaObjectiveUpdated(const FText& ObjectiveText, float Progress);

	UFUNCTION()
	void HandleControllerArenaObjectiveCompleted();

	UFUNCTION()
	void HandleControllerEventFeedEntryAdded(const FText& Text, float Duration);

	UFUNCTION()
	void HandleControllerEventFeedCleared();

	UFUNCTION()
	void HandleControllerUltimateReadyPromptShown();

	UFUNCTION()
	void HandleControllerUltimateReadyPromptHidden();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD", meta = (BindWidgetOptional))
	TObjectPtr<UDBAPlayerUnitFrameWidgetBase> PlayerUnitFrame;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD", meta = (BindWidgetOptional))
	TObjectPtr<UDBAAbilityBarWidgetBase> AbilityBar;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD", meta = (BindWidgetOptional))
	TObjectPtr<UDBAPassiveAndResonancePanelWidgetBase> PassiveAndResonancePanel;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD", meta = (BindWidgetOptional))
	TObjectPtr<UDBABuffBarWidgetBase> BuffBar;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD", meta = (BindWidgetOptional))
	TObjectPtr<UDBADebuffBarWidgetBase> DebuffBar;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD", meta = (BindWidgetOptional))
	TObjectPtr<UDBACCBarWidgetBase> CCBar;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD", meta = (BindWidgetOptional))
	TObjectPtr<UDBASelfCastBarWidgetBase> SelfCastBar;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD", meta = (BindWidgetOptional))
	TObjectPtr<UDBAMomentumPanelWidgetBase> MomentumPanel;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD", meta = (BindWidgetOptional))
	TObjectPtr<UDBAChainUltimatePanelWidgetBase> ChainUltimatePanel;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD", meta = (BindWidgetOptional))
	TObjectPtr<UDBACombatAnnouncementWidgetBase> CombatAnnouncement;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD", meta = (BindWidgetOptional))
	TObjectPtr<UDBACriticalStateHintWidgetBase> CriticalStateHint;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD", meta = (BindWidgetOptional))
	TObjectPtr<UDBAUltimateReadyPromptWidgetBase> UltimateReadyPrompt;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD", meta = (BindWidgetOptional))
	TObjectPtr<UDBAConnectionWarningWidgetBase> ConnectionWarning;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD", meta = (BindWidgetOptional))
	TObjectPtr<UDBAArenaObjectiveTrackerWidgetBase> ObjectiveTracker;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD", meta = (BindWidgetOptional))
	TObjectPtr<UDBAArenaEventFeedWidgetBase> EventFeed;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD")
	TObjectPtr<UDBAArenaHUDWidgetController> WidgetController;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD")
	TObjectPtr<UDBAPlayerUnitFrameWidgetController> PlayerUnitFrameWidgetController;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD")
	TWeakObjectPtr<ADBAZodiacCharacterBase> BoundArenaHUDCharacter;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|ArenaHUD")
	bool bIsEditMode;
};
