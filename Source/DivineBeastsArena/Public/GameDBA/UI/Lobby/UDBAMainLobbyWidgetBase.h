// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "GameDBA/Core/DBAEnumsCore.h"
#include "UDBAMainLobbyWidgetBase.generated.h"

class UDBAMainLobbyWidgetController;
class UDBAPartyPanelWidgetBase;
class UDBAQueueModeSelectWidgetBase;

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

protected:
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
};
