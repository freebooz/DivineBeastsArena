// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "GameDBA/Core/DBAEnumsCore.h"
#include "UDBAMainLobbyWidgetBase.generated.h"

class UDBAMainLobbyWidgetController;
class UDBAPartyPanelWidgetBase;
class UDBAQueueModeSelectWidgetBase;
class UButton;
class UEditableTextBox;
class UTextBlock;
struct FDBALobbyPlayerSummary;
enum class EDBALobbyBackendState : uint8;

/**
 * DBAMainLobbyWidgetBase
 *
 * 主大厅界面 Widget 基类
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAMainLobbyWidgetBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBAMainLobbyWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void TryBindWidgetController();

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby")
	void SetWidgetController(UDBAMainLobbyWidgetController* InController);

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby")
	void RefreshPartyInfo();

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby")
	void SwitchFiveCampTheme(EDBAFiveCamp FiveCamp);

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby")
	void NavigateToNewbieVillage();

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby")
	void NavigateToPractice();

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby")
	void OpenQueueModeSelect();

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby")
	void OpenFriendList();

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby")
	void OpenSettings();

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby")
	void ExitGame();

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby|Backend")
	void BackendRefreshPlayerData();

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby|Backend")
	void BackendRefreshRoomList();

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby|Backend")
	void BackendCreateRoom();

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby|Backend")
	void BackendJoinRoom(const FString& RoomId);

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby|Backend")
	void BackendSetReady(bool bReady);

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby|Backend")
	void BackendStartRoom();

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby|Backend")
	void BackendStartMatchmaking(const FString& Mode, const FString& RegionCode);

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby|Backend")
	void BackendCancelMatchmaking();

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby|Backend")
	void BackendJoinRoomFromInput();

protected:
	UFUNCTION()
	void HandleBackendStateChanged(EDBALobbyBackendState NewState);

	UFUNCTION()
	void HandleBackendError(const FString& ErrorMessage);

	UFUNCTION()
	void HandlePlayerSummaryUpdated(const FDBALobbyPlayerSummary& Summary);

	UFUNCTION()
	void HandleCreateRoomClicked();

	UFUNCTION()
	void HandleRefreshRoomsClicked();

	UFUNCTION()
	void HandleJoinRoomClicked();

	UFUNCTION()
	void HandleReadyClicked();

	UFUNCTION()
	void HandleStartGameClicked();

	UFUNCTION()
	void HandleStartMatchClicked();

	UFUNCTION()
	void HandleCancelMatchClicked();

	void BindBackendUiControls();
	void BindControllerDelegates();
	void UnbindControllerDelegates();
	void UpdateBackendStateText(EDBALobbyBackendState NewState);
	void UpdateBackendButtonsState(EDBALobbyBackendState NewState);
	void UpdatePlayerSummaryText(const FDBALobbyPlayerSummary& Summary);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|MainLobby", meta = (DisplayName = "On Party Info Refreshed"))
	void BP_OnPartyInfoRefreshed();

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|MainLobby", meta = (DisplayName = "On FiveCamp Theme Switched"))
	void BP_OnFiveCampThemeSwitched(EDBAFiveCamp NewTheme);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBA|MainLobby")
	TObjectPtr<UDBAMainLobbyWidgetController> WidgetController;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|MainLobby", meta = (BindWidgetOptional))
	TObjectPtr<UDBAPartyPanelWidgetBase> PartyPanel;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|MainLobby", meta = (BindWidgetOptional))
	TObjectPtr<UDBAQueueModeSelectWidgetBase> QueueModeSelect;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|MainLobby")
	EDBAFiveCamp CurrentFiveCampTheme;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|MainLobby", meta = (BindWidgetOptional))
	TObjectPtr<UButton> CreateRoomButton;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|MainLobby", meta = (BindWidgetOptional))
	TObjectPtr<UButton> RefreshRoomsButton;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|MainLobby", meta = (BindWidgetOptional))
	TObjectPtr<UButton> JoinRoomButton;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|MainLobby", meta = (BindWidgetOptional))
	TObjectPtr<UButton> ReadyButton;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|MainLobby", meta = (BindWidgetOptional))
	TObjectPtr<UButton> StartGameButton;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|MainLobby", meta = (BindWidgetOptional))
	TObjectPtr<UButton> StartMatchButton;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|MainLobby", meta = (BindWidgetOptional))
	TObjectPtr<UButton> CancelMatchButton;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|MainLobby", meta = (BindWidgetOptional))
	TObjectPtr<UEditableTextBox> JoinRoomIdInput;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|MainLobby", meta = (BindWidgetOptional))
	TObjectPtr<UEditableTextBox> MatchModeInput;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|MainLobby", meta = (BindWidgetOptional))
	TObjectPtr<UEditableTextBox> MatchRegionInput;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|MainLobby", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PlayerNameText;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|MainLobby", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PlayerLevelText;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|MainLobby", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PlayerExperienceText;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|MainLobby", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PlayerGoldText;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|MainLobby", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PlayerTicketsText;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|MainLobby", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> BackendStateText;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|MainLobby", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> BackendErrorText;
};
