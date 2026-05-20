// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/UI/DBAWidgetController.h"
#include "GameCore/Account/DBAAccountTypes.h"
#include "GameCore/Party/DBAPartyTypes.h"
#include "UDBAMainLobbyWidgetController.generated.h"

class UGameBackendClientSubsystem;

UENUM(BlueprintType)
enum class EDBALobbyBackendState : uint8
{
	Idle,
	Loading,
	InRoom,
	Ready,
	Matching,
	MatchFound,
	Connecting,
	Error
};

USTRUCT(BlueprintType)
struct FDBALobbyPlayerSummary
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "DBA|MainLobby|Backend")
	FString PlayerId;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|MainLobby|Backend")
	FString DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|MainLobby|Backend")
	int32 Level = 0;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|MainLobby|Backend")
	int32 Experience = 0;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|MainLobby|Backend")
	int32 Gold = 0;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|MainLobby|Backend")
	int32 Tickets = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyBackendStateChanged, EDBALobbyBackendState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyBackendError, const FString&, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyRawDataUpdated, const FString&, DataJson);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyPlayerSummaryUpdated, const FDBALobbyPlayerSummary&, PlayerSummary);

UCLASS(BlueprintType)
class DIVINEBEASTSARENA_API UDBAMainLobbyWidgetController : public UDBAWidgetController
{
	GENERATED_BODY()

public:
	UDBAMainLobbyWidgetController(const FObjectInitializer& ObjectInitializer);

public:
	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby")
	void RequestPartyInfo();

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby")
	void RequestSwitchFiveCampTheme(uint8 FiveCamp);

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby")
	void RequestNavigateToNewbieVillage();

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby")
	void RequestNavigateToPractice();

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby")
	void RequestExitGame();

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby|Backend")
	void InitializeBackendLobby();

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby|Backend")
	void RefreshPlayerData();

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby|Backend")
	void RefreshRoomList();

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby|Backend")
	void CreateRoom();

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby|Backend")
	void JoinRoom(const FString& RoomId);

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby|Backend")
	void LeaveRoom();

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby|Backend")
	void SetReady(bool bReady);

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby|Backend")
	void StartRoom();

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby|Backend")
	void StartMatchmaking(const FString& Mode, const FString& RegionCode);

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby|Backend")
	void CancelMatchmaking();

	UFUNCTION(BlueprintCallable, Category = "DBA|MainLobby|Backend")
	void NotifyMatchFinishedClientView();

	UFUNCTION(BlueprintPure, Category = "DBA|MainLobby|Backend")
	EDBALobbyBackendState GetBackendState() const { return BackendState; }

	UFUNCTION(BlueprintPure, Category = "DBA|MainLobby|Backend")
	FString GetCurrentRoomId() const { return CurrentRoomId; }

	UFUNCTION(BlueprintPure, Category = "DBA|MainLobby|Backend")
	FString GetCurrentTicketId() const { return CurrentTicketId; }

	UFUNCTION(BlueprintPure, Category = "DBA|MainLobby|Backend")
	const FDBALobbyPlayerSummary& GetPlayerSummary() const { return PlayerSummary; }

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPartyInfoReady, const FDBAPartyInfo&, PartyInfo);
	UPROPERTY(BlueprintAssignable, Category = "DBA|MainLobby")
	FOnPartyInfoReady OnPartyInfoReady;

	UPROPERTY(BlueprintAssignable, Category = "DBA|MainLobby|Backend")
	FOnLobbyBackendStateChanged OnBackendStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "DBA|MainLobby|Backend")
	FOnLobbyBackendError OnBackendError;

	UPROPERTY(BlueprintAssignable, Category = "DBA|MainLobby|Backend")
	FOnLobbyRawDataUpdated OnProfileUpdated;

	UPROPERTY(BlueprintAssignable, Category = "DBA|MainLobby|Backend")
	FOnLobbyRawDataUpdated OnInventoryUpdated;

	UPROPERTY(BlueprintAssignable, Category = "DBA|MainLobby|Backend")
	FOnLobbyRawDataUpdated OnRoomsUpdated;

	UPROPERTY(BlueprintAssignable, Category = "DBA|MainLobby|Backend")
	FOnLobbyRawDataUpdated OnMatchTicketUpdated;

	UPROPERTY(BlueprintAssignable, Category = "DBA|MainLobby|Backend")
	FOnLobbyPlayerSummaryUpdated OnPlayerSummaryUpdated;

private:
	UFUNCTION()
	void HandleGetProfileResponse(bool bSuccess, const FString& ErrorMessage, const FString& DataJson);

	UFUNCTION()
	void HandleGetInventoryResponse(bool bSuccess, const FString& ErrorMessage, const FString& DataJson);

	UFUNCTION()
	void HandleGetRoomsResponse(bool bSuccess, const FString& ErrorMessage, const FString& DataJson);

	UFUNCTION()
	void HandleCreateRoomResponse(bool bSuccess, const FString& ErrorMessage, const FString& DataJson);

	UFUNCTION()
	void HandleJoinRoomResponse(bool bSuccess, const FString& ErrorMessage, const FString& DataJson);

	UFUNCTION()
	void HandleLeaveRoomResponse(bool bSuccess, const FString& ErrorMessage, const FString& DataJson);

	UFUNCTION()
	void HandleSetReadyResponse(bool bSuccess, const FString& ErrorMessage, const FString& DataJson);

	UFUNCTION()
	void HandleStartRoomResponse(bool bSuccess, const FString& ErrorMessage, const FString& DataJson);

	UFUNCTION()
	void HandleCreateTicketResponse(bool bSuccess, const FString& ErrorMessage, const FString& DataJson);

	UFUNCTION()
	void HandleGetTicketResponse(bool bSuccess, const FString& ErrorMessage, const FString& DataJson);

	UFUNCTION()
	void HandleCancelTicketResponse(bool bSuccess, const FString& ErrorMessage, const FString& DataJson);

	UFUNCTION()
	void HandleGetSessionResponse(bool bSuccess, const FString& ErrorMessage, const FString& DataJson);

	UFUNCTION()
	void HandleGetConnectionResponse(bool bSuccess, const FString& ErrorMessage, const FString& DataJson);

	UFUNCTION()
	void HandleConnectToServerResponse(bool bSuccess, const FString& ErrorMessage, const FString& DataJson);

	void UpdatePlayerSummaryFromProfile(const FString& ProfileJson);
	void UpdatePlayerSummaryFromInventory(const FString& InventoryJson);
	void ResolveStateAfterBackgroundRefresh();
	void SetBackendState(EDBALobbyBackendState NewState);
	void ReportBackendError(const FString& ErrorMessage);
	void StartTicketPolling();
	void StopTicketPolling();
	void PollTicketOnce();
	void ResolveMatchAndConnect(const FString& TicketDataJson);
	void TrackTelemetry(const FString& EventName, const TMap<FString, FString>& Properties) const;
	FString ExtractStringByKeys(const FString& Json, const TArray<FString>& Keys) const;
	bool IsTicketMatched(const FString& TicketDataJson, FString& OutSessionId) const;
	UGameBackendClientSubsystem* GetBackendSubsystem() const;

private:
	UPROPERTY(Transient)
	EDBALobbyBackendState BackendState = EDBALobbyBackendState::Idle;

	UPROPERTY(Transient)
	FString CurrentRoomId;

	UPROPERTY(Transient)
	FString CurrentTicketId;

	UPROPERTY(Transient)
	FString CurrentSessionId;

	UPROPERTY(Transient)
	FDBALobbyPlayerSummary PlayerSummary;

	UPROPERTY(Transient)
	bool bReadyLocalState = false;

	UPROPERTY(Transient)
	bool bProfileLoaded = false;

	UPROPERTY(Transient)
	bool bInventoryLoaded = false;

	UPROPERTY(Transient)
	FString PendingJoinRoomId;

	UPROPERTY(Transient)
	bool bPendingReadyState = false;

	UPROPERTY(Transient)
	FString PendingMatchMode;

	UPROPERTY(Transient)
	FString PendingMatchRegion;

	FTimerHandle TicketPollingTimerHandle;
};
