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
#include "GameCore/Session/DBALoginFlowSubsystem.h"
#include "GameCore/Subsystems/DBAGameInstanceSubsystemBase.h"
#include "GameDBA/UI/Lobby/UDBAMainLobbyWidgetBase.h"
#include "GameDBA/UI/Lobby/UDBAInteractionPromptWidgetBase.h"
#include "GameDBA/UI/Lobby/Login/UDBALoginFlowWidgetBase.h"
#include "GameDBA/UI/Lobby/Login/UDBACharacterSelectFlowWidgetBase.h"
#include "GameDBA/UI/Lobby/Login/UDBACharacterCreateFlowWidgetBase.h"
#include "DBAGameUIManager.generated.h"

class UDBAArenaHUDRootWidgetBase;
class UDBAArenaHUDWidgetController;
class ADBAZodiacCharacterBase;
class UDBAMainLobbyWidgetBase;
class UDBALobbyPlayerHUDWidgetBase;
class UDBALoadingScreenWidgetBase;
class UDBAGameSettingsWidgetBase;
class UDBAInventoryWidgetBase;
class UDBAPartyPanelWidgetBase;
class UDBAInvitePanelWidgetBase;
class UDBAQueueModeSelectWidgetBase;
class UDBAQueueStatusWidgetBase;
class UDBAReadyCheckWidgetBase;
class UDBAMatchFoundWidgetBase;
class UDBAPortalConfirmWidgetBase;
class UDBANewbieVillageMainWidgetBase;
class UDBANewbieTaskTrackerWidgetBase;
class UAudioComponent;
class USoundBase;

/**
 * EDBAUIState
 * UI状态枚举
 */
UENUM(BlueprintType)
enum class EDBAUIState : uint8
{
	None,
	MainMenu,
	Lobby,
	HeroSelect,
	Loading,
	InGame,
	Pause
};

/**
 * FOnUIStateChanged
 * UI状态改变委托
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUIStateChanged, EDBAUIState, NewState);

/**
 * DBAGameUIManager
 *
 * 游戏UI状态管理器
 * 统一管理所有游戏界面的显示/隐藏和切换
 */
UCLASS(Blueprintable)
class DIVINEBEASTSARENA_API UDBAGameUIManager : public UDBAGameInstanceSubsystemBase
{
	GENERATED_BODY()

public:
	UDBAGameUIManager();

protected:
	virtual void OnSubsystemInitialize() override;
	virtual void OnSubsystemDeinitialize() override;

public:
	/** 获取当前状态 */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	EDBAUIState GetCurrentState() const { return CurrentState; }

	/** 切换UI状态 */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	void TransitionTo(EDBAUIState NewState);

	/** 注册状态改变回调 (C++内部使用) */
	void RegisterStateChangeCallback(const FOnUIStateChanged& Delegate);

	/** 显示主大厅 */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	void ShowMainLobby();

	/** 隐藏主大厅 */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	void HideMainLobby();

	/** 显示大厅玩家HUD */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	void ShowLobbyPlayerHUD();

	/** 隐藏大厅玩家HUD */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	void HideLobbyPlayerHUD();

	/** 显示战斗HUD */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	void ShowArenaHUD();

	/** 隐藏战斗HUD */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	void HideArenaHUD();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|ArenaHUD")
	void UpdateArenaHUDPlayerVitals(float CurrentHP, float MaxHP, float CurrentEnergy, float MaxEnergy);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|ArenaHUD")
	void UpdateArenaHUDPlayerLevel(int32 Level);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|ArenaHUD")
	void UpdateArenaHUDUltimateEnergy(float CurrentEnergy, float MaxEnergy);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|ArenaHUD")
	void UpdateArenaHUDCombatState(int32 ChainLevel, int32 ResonanceLevel);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|ArenaHUD")
	void UpdateArenaHUDMomentum(int32 MomentumLevel, float MomentumProgress);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|ArenaHUD|Status")
	void AddArenaHUDBuff(const FString& BuffId, float Duration);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|ArenaHUD|Status")
	void RemoveArenaHUDBuff(const FString& BuffId);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|ArenaHUD|Status")
	void ClearArenaHUDBuffs();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|ArenaHUD|Status")
	void AddArenaHUDDebuff(const FString& DebuffId, float Duration);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|ArenaHUD|Status")
	void RemoveArenaHUDDebuff(const FString& DebuffId);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|ArenaHUD|Status")
	void ClearArenaHUDDebuffs();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|ArenaHUD|Status")
	void AddArenaHUDCCEffect(const FString& CCId, float Duration);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|ArenaHUD|Status")
	void RemoveArenaHUDCCEffect(const FString& CCId);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|ArenaHUD|Status")
	void ClearArenaHUDCCEffects();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|ArenaHUD|Feedback")
	void ShowArenaHUDCombatAnnouncement(const FText& Text, float Duration);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|ArenaHUD|Feedback")
	void ClearArenaHUDCombatAnnouncement();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|ArenaHUD|Feedback")
	void UpdateArenaHUDCriticalStateHints(bool bLowHP, bool bLowEnergy);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|ArenaHUD|Feedback")
	void UpdateArenaHUDObjective(const FText& ObjectiveText, float Progress);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|ArenaHUD|Feedback")
	void CompleteArenaHUDObjective();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|ArenaHUD|Feedback")
	void AddArenaHUDEventFeedEntry(const FText& Text, float Duration);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|ArenaHUD|Feedback")
	void ClearArenaHUDEventFeed();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|ArenaHUD|Feedback")
	void ShowArenaHUDUltimateReadyPrompt();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|ArenaHUD|Feedback")
	void HideArenaHUDUltimateReadyPrompt();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|ArenaHUD")
	void BindArenaHUDToCharacter(ADBAZodiacCharacterBase* Character);

	/** 清理所有UI */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	void ClearAllUI();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	void ShowLobbyLoadingScreen();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	void HideLobbyLoadingScreen();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	void ShowGameSettings();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	void HideGameSettings();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	void ToggleGameSettings();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	bool IsGameSettingsVisible() const { return bGameSettingsVisible; }

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	void ShowInventory();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	void HideInventory();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	void ToggleInventory();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	bool IsInventoryVisible() const { return bInventoryVisible; }

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|Lobby")
	void ShowPartyPanel();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|Lobby")
	void HidePartyPanel();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|Lobby")
	void TogglePartyPanel();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|Lobby")
	void ShowInvitePanel();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|Lobby")
	void HideInvitePanel();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|Lobby")
	void ToggleInvitePanel();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|Lobby")
	void ShowQueueModeSelect();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|Lobby")
	void HideQueueModeSelect();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|Lobby")
	void ToggleQueueModeSelect();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|Lobby")
	void ShowQueueStatus(const FText& ModeName, const FText& MapName, const FText& EstimatedWaitTime);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|Lobby")
	void HideQueueStatus();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|Lobby")
	void ShowMatchFound(const FText& ModeName, const FText& MapName);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|Lobby")
	void HideMatchFound();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|Lobby")
	void ShowReadyCheck(const FText& ModeName, const FText& MapName, float TimeoutSeconds = 20.0f);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|Lobby")
	void HideReadyCheck();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|Lobby")
	void ShowPortalConfirm(FName DestinationId, const FText& DestinationName, const FText& DestinationDescription, bool bCanTeleport, const FText& ConditionText);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|Lobby")
	void HidePortalConfirm();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|Lobby")
	void ShowInteractionPrompt(EDBAInteractionType Type, const FText& ObjectName, const FText& PromptText, bool bCanInteract);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|Lobby")
	void HideInteractionPrompt();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|Lobby")
	void UpdateInteractionProgress(float Progress);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|Lobby")
	void ShowNewbieVillageMain();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|Lobby")
	void HideNewbieVillageMain();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|Lobby")
	void ShowNewbieTaskTracker();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|Lobby")
	void HideNewbieTaskTracker();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager|Lobby")
	void ToggleNewbieTaskTracker();

	/** 外部请求显示登录流程界面 */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	void RequestShowLoginFlowWidget();

protected:
	/** 创建Lobby Widget */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	virtual void CreateMainLobbyWidget();

	/** 创建大厅玩家HUD Widget */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	virtual void CreateLobbyPlayerHUDWidget();

	/** 创建Arena HUD Widget */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	virtual void CreateArenaHUDWidget();

	APlayerController* GetArenaHUDLocalPlayerController() const;
	UDBAArenaHUDWidgetController* EnsureArenaHUDWidgetController(class APlayerController* InPlayerController);

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	virtual void CreateGameSettingsWidget();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	virtual void CreateInventoryWidget();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	virtual void CreatePartyPanelWidget();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	virtual void CreateInvitePanelWidget();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	virtual void CreateQueueModeSelectWidget();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	virtual void CreateQueueStatusWidget();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	virtual void CreateReadyCheckWidget();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	virtual void CreateMatchFoundWidget();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	virtual void CreatePortalConfirmWidget();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	virtual void CreateInteractionPromptWidget();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	virtual void CreateNewbieVillageMainWidget();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	virtual void CreateNewbieTaskTrackerWidget();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	virtual void ShowLoginFlowWidget();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	virtual void HideLoginFlowWidget();

protected:
	/** 当前UI状态 */
	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Manager")
	EDBAUIState CurrentState = EDBAUIState::None;

	/** 状态改变回调 */
	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Manager")
	FOnUIStateChanged OnStateChanged;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Manager")
	TObjectPtr<UDBAMainLobbyWidgetBase> MainLobbyWidget;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Manager")
	TObjectPtr<UDBALobbyPlayerHUDWidgetBase> LobbyPlayerHUDWidget;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Manager")
	TObjectPtr<UDBAArenaHUDRootWidgetBase> ArenaHUDWidget;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Manager")
	TObjectPtr<UDBAArenaHUDWidgetController> ArenaHUDWidgetController;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Manager")
	TWeakObjectPtr<ADBAZodiacCharacterBase> ArenaHUDCharacter;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Manager")
	TObjectPtr<UDBALoginFlowWidgetBase> LoginWidget;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Manager")
	TObjectPtr<UDBACharacterSelectFlowWidgetBase> CharacterSelectWidget;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Manager")
	TObjectPtr<UDBACharacterCreateFlowWidgetBase> CharacterCreateWidget;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Manager")
	TObjectPtr<UDBALoadingScreenWidgetBase> LobbyLoadingWidget;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Manager")
	TObjectPtr<UDBAGameSettingsWidgetBase> GameSettingsWidget;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Manager")
	TObjectPtr<UDBAInventoryWidgetBase> InventoryWidget;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Manager")
	TObjectPtr<UDBAPartyPanelWidgetBase> PartyPanelWidget;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Manager")
	TObjectPtr<UDBAInvitePanelWidgetBase> InvitePanelWidget;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Manager")
	TObjectPtr<UDBAQueueModeSelectWidgetBase> QueueModeSelectWidget;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Manager")
	TObjectPtr<UDBAQueueStatusWidgetBase> QueueStatusWidget;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Manager")
	TObjectPtr<UDBAReadyCheckWidgetBase> ReadyCheckWidget;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Manager")
	TObjectPtr<UDBAMatchFoundWidgetBase> MatchFoundWidget;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Manager")
	TObjectPtr<UDBAPortalConfirmWidgetBase> PortalConfirmWidget;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Manager")
	TObjectPtr<UDBAInteractionPromptWidgetBase> InteractionPromptWidget;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Manager")
	TObjectPtr<UDBANewbieVillageMainWidgetBase> NewbieVillageMainWidget;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Manager")
	TObjectPtr<UDBANewbieTaskTrackerWidgetBase> NewbieTaskTrackerWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Manager")
	TSubclassOf<UDBAMainLobbyWidgetBase> MainLobbyWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Manager")
	TSubclassOf<UDBALobbyPlayerHUDWidgetBase> LobbyPlayerHUDWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Manager")
	TSubclassOf<UDBAArenaHUDRootWidgetBase> ArenaHUDWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Manager")
	TSubclassOf<UDBALoginFlowWidgetBase> LoginWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Manager")
	TSubclassOf<UDBACharacterSelectFlowWidgetBase> CharacterSelectWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Manager")
	TSubclassOf<UDBACharacterCreateFlowWidgetBase> CharacterCreateWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Manager")
	TSubclassOf<UDBALoadingScreenWidgetBase> LobbyLoadingWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Manager")
	TSubclassOf<UDBAGameSettingsWidgetBase> GameSettingsWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Manager")
	TSubclassOf<UDBAInventoryWidgetBase> InventoryWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Manager")
	TSubclassOf<UDBAPartyPanelWidgetBase> PartyPanelWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Manager")
	TSubclassOf<UDBAInvitePanelWidgetBase> InvitePanelWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Manager")
	TSubclassOf<UDBAQueueModeSelectWidgetBase> QueueModeSelectWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Manager")
	TSubclassOf<UDBAQueueStatusWidgetBase> QueueStatusWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Manager")
	TSubclassOf<UDBAReadyCheckWidgetBase> ReadyCheckWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Manager")
	TSubclassOf<UDBAMatchFoundWidgetBase> MatchFoundWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Manager")
	TSubclassOf<UDBAPortalConfirmWidgetBase> PortalConfirmWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Manager")
	TSubclassOf<UDBAInteractionPromptWidgetBase> InteractionPromptWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Manager")
	TSubclassOf<UDBANewbieVillageMainWidgetBase> NewbieVillageMainWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Manager")
	TSubclassOf<UDBANewbieTaskTrackerWidgetBase> NewbieTaskTrackerWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Manager")
	TObjectPtr<class UDBASplashVideoWidget> SplashVideoWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI|Manager")
	TSubclassOf<class UDBASplashVideoWidget> SplashVideoWidgetClass;

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	void ShowSplashVideo();

	UFUNCTION(BlueprintCallable, Category = "DBA|UI|Manager")
	void HideSplashVideo();

private:
	UPROPERTY(Transient)
	EDBALoginFlowState CachedLoginFlowState = EDBALoginFlowState::Startup;

	UFUNCTION()
	void HandleLoginFlowStateChanged(EDBALoginFlowState NewState);

	void RefreshLoginFlowWidgetVisibility();
	void HideAllFlowWidgets();
	void RestoreInputModeAfterOverlayClosed();
	void EnsureLoginFlowBackgroundMusic();
	void StopLoginFlowBackgroundMusic();
	void ScheduleFlowWidgetRefreshRetry();
	void ResetFlowWidgetRefreshRetry();
	void HandleFlowWidgetRefreshRetry();
	void ScheduleLobbyHUDRefreshRetry();
	void ResetLobbyHUDRefreshRetry();

	UFUNCTION()
	void HandleLoginFlowBackgroundMusicFinished();

	UFUNCTION()
	void HandleReadyCheckCompleted(bool bAccepted);

	UFUNCTION()
	void HandlePortalConfirmed(FName DestinationId);

	UFUNCTION()
	void HandlePortalCancelled();

	template<typename WidgetType>
	WidgetType* EnsureFlowWidgetCreated(TSubclassOf<WidgetType> WidgetClass, TObjectPtr<WidgetType>& WidgetInstance);
	void SetFlowWidgetVisible(UUserWidget* WidgetToShow);

	void TryShowSplashVideo();
	void EnsureLoginFlowStartedFromManager();

	bool bMainLobbyVisible = false;
	bool bLobbyPlayerHUDVisible = false;
	bool bArenaHUDVisible = false;
	bool bFlowWidgetVisible = false;
	bool bLobbyLoadingVisible = false;
	bool bGameSettingsVisible = false;
	bool bInventoryVisible = false;
	bool bPartyPanelVisible = false;
	bool bInvitePanelVisible = false;
	bool bQueueModeSelectVisible = false;
	bool bQueueStatusVisible = false;
	bool bReadyCheckVisible = false;
	bool bMatchFoundVisible = false;
	bool bPortalConfirmVisible = false;
	bool bInteractionPromptVisible = false;
	bool bNewbieVillageMainVisible = false;
	bool bNewbieTaskTrackerVisible = false;
	bool bLoginFlowStartRequested = false;
	bool bIsDeinitializing = false;
	int32 FlowWidgetRefreshRetryCount = 0;
	int32 LobbyHUDRefreshRetryCount = 0;

	FTimerHandle SplashVideoTimerHandle;
	FTimerHandle FlowWidgetRefreshRetryTimerHandle;
	FTimerHandle LobbyHUDRefreshRetryTimerHandle;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> LoginFlowBackgroundMusicComponent;

	UPROPERTY(EditDefaultsOnly, Category = "DBA|UI|Audio")
	TObjectPtr<USoundBase> LoginFlowBackgroundMusicSound;
};
