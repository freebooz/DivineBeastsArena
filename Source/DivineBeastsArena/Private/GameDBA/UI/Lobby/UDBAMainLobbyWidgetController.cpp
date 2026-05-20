// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Lobby/UDBAMainLobbyWidgetController.h"

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "GameBackendClientSubsystem.h"
#include "GameBackendMatchService.h"
#include "GameBackendPlayerService.h"
#include "GameBackendRoomService.h"
#include "GameBackendSessionService.h"
#include "GameBackendTelemetryService.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "TimerManager.h"

namespace
{
	bool TryParseJsonObject(const FString& Json, TSharedPtr<FJsonObject>& OutRoot)
	{
		OutRoot.Reset();
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
		return FJsonSerializer::Deserialize(Reader, OutRoot) && OutRoot.IsValid();
	}

	FString ReadStringByKeysDeep(const TSharedPtr<FJsonObject>& Obj, const TArray<FString>& Keys, int32 Depth = 0)
	{
		if (!Obj.IsValid() || Depth > 8)
		{
			return FString();
		}

		for (const FString& Key : Keys)
		{
			FString Value;
			if (Obj->TryGetStringField(Key, Value) && !Value.IsEmpty())
			{
				return Value;
			}
		}

		for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : Obj->Values)
		{
			if (!Pair.Value.IsValid())
			{
				continue;
			}

			if (Pair.Value->Type == EJson::Object)
			{
				const FString NestedValue = ReadStringByKeysDeep(Pair.Value->AsObject(), Keys, Depth + 1);
				if (!NestedValue.IsEmpty())
				{
					return NestedValue;
				}
				continue;
			}

			if (Pair.Value->Type == EJson::Array)
			{
				const TArray<TSharedPtr<FJsonValue>>& ArrayValues = Pair.Value->AsArray();
				for (const TSharedPtr<FJsonValue>& Item : ArrayValues)
				{
					if (Item.IsValid() && Item->Type == EJson::Object)
					{
						const FString NestedValue = ReadStringByKeysDeep(Item->AsObject(), Keys, Depth + 1);
						if (!NestedValue.IsEmpty())
						{
							return NestedValue;
						}
					}
				}
			}
		}

		return FString();
	}

	int32 ReadIntByKeysDeep(const TSharedPtr<FJsonObject>& Obj, const TArray<FString>& Keys, int32 Depth = 0)
	{
		if (!Obj.IsValid() || Depth > 8)
		{
			return 0;
		}

		for (const FString& Key : Keys)
		{
			double NumberValue = 0.0;
			if (Obj->TryGetNumberField(Key, NumberValue))
			{
				return FMath::RoundToInt(NumberValue);
			}

			FString StringValue;
			if (Obj->TryGetStringField(Key, StringValue) && !StringValue.IsEmpty())
			{
				return FCString::Atoi(*StringValue);
			}
		}

		for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : Obj->Values)
		{
			if (!Pair.Value.IsValid())
			{
				continue;
			}

			if (Pair.Value->Type == EJson::Object)
			{
				const int32 NestedValue = ReadIntByKeysDeep(Pair.Value->AsObject(), Keys, Depth + 1);
				if (NestedValue != 0)
				{
					return NestedValue;
				}
				continue;
			}

			if (Pair.Value->Type == EJson::Array)
			{
				const TArray<TSharedPtr<FJsonValue>>& ArrayValues = Pair.Value->AsArray();
				for (const TSharedPtr<FJsonValue>& Item : ArrayValues)
				{
					if (Item.IsValid() && Item->Type == EJson::Object)
					{
						const int32 NestedValue = ReadIntByKeysDeep(Item->AsObject(), Keys, Depth + 1);
						if (NestedValue != 0)
						{
							return NestedValue;
						}
					}
				}
			}
		}

		return 0;
	}
}

UDBAMainLobbyWidgetController::UDBAMainLobbyWidgetController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDBAMainLobbyWidgetController::RequestPartyInfo()
{
	FDBAPartyInfo StubPartyInfo;
	StubPartyInfo.PartyId = FDBAPartyId(TEXT("party_001"));
	StubPartyInfo.LeaderAccountId = FDBAAccountId(TEXT("player_001"));
	OnPartyInfoReady.Broadcast(StubPartyInfo);
}

void UDBAMainLobbyWidgetController::RequestSwitchFiveCampTheme(uint8 FiveCamp)
{
	UE_LOG(LogDBAUI, Log, TEXT("[MainLobby] Switch camp theme request: %d"), FiveCamp);
}

void UDBAMainLobbyWidgetController::RequestNavigateToNewbieVillage()
{
	UE_LOG(LogDBAUI, Log, TEXT("[MainLobby] Navigate to newbie village requested."));
}

void UDBAMainLobbyWidgetController::RequestNavigateToPractice()
{
	UE_LOG(LogDBAUI, Log, TEXT("[MainLobby] Navigate to practice requested."));
}

void UDBAMainLobbyWidgetController::RequestExitGame()
{
	UE_LOG(LogDBAUI, Log, TEXT("[MainLobby] Exit game requested."));
}

void UDBAMainLobbyWidgetController::InitializeBackendLobby()
{
	CurrentRoomId.Empty();
	CurrentTicketId.Empty();
	CurrentSessionId.Empty();
	bReadyLocalState = false;
	bProfileLoaded = false;
	bInventoryLoaded = false;
	SetBackendState(EDBALobbyBackendState::Idle);

	RefreshPlayerData();
	RefreshRoomList();
}

void UDBAMainLobbyWidgetController::RefreshPlayerData()
{
	UGameBackendClientSubsystem* Backend = GetBackendSubsystem();
	UGameBackendPlayerService* PlayerService = Backend ? Backend->GetPlayerService() : nullptr;
	if (!PlayerService)
	{
		ReportBackendError(TEXT("Player service unavailable."));
		return;
	}

	SetBackendState(EDBALobbyBackendState::Loading);
	bProfileLoaded = false;
	bInventoryLoaded = false;

	FGameBackendResponseDelegate Callback;
	Callback.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UDBAMainLobbyWidgetController, HandleGetProfileResponse));
	PlayerService->GetMyProfile(Callback);
}

void UDBAMainLobbyWidgetController::RefreshRoomList()
{
	UGameBackendClientSubsystem* Backend = GetBackendSubsystem();
	UGameBackendRoomService* RoomService = Backend ? Backend->GetRoomService() : nullptr;
	if (!RoomService)
	{
		ReportBackendError(TEXT("Room service unavailable."));
		return;
	}

	SetBackendState(EDBALobbyBackendState::Loading);
	FGameBackendResponseDelegate Callback;
	Callback.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UDBAMainLobbyWidgetController, HandleGetRoomsResponse));
	RoomService->GetRooms(Callback);
}

void UDBAMainLobbyWidgetController::CreateRoom()
{
	UGameBackendClientSubsystem* Backend = GetBackendSubsystem();
	UGameBackendRoomService* RoomService = Backend ? Backend->GetRoomService() : nullptr;
	if (!RoomService)
	{
		ReportBackendError(TEXT("Room service unavailable."));
		return;
	}

	SetBackendState(EDBALobbyBackendState::Loading);

	FGameBackendRoomCreateRequest Request;
	Request.Mode = TEXT("default");
	Request.Region = TEXT("local");
	Request.MaxPlayers = 10;
	Request.bPrivate = false;

	FGameBackendResponseDelegate Callback;
	Callback.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UDBAMainLobbyWidgetController, HandleCreateRoomResponse));
	RoomService->CreateRoom(Request, Callback);
}

void UDBAMainLobbyWidgetController::JoinRoom(const FString& RoomId)
{
	UGameBackendClientSubsystem* Backend = GetBackendSubsystem();
	UGameBackendRoomService* RoomService = Backend ? Backend->GetRoomService() : nullptr;
	if (!RoomService)
	{
		ReportBackendError(TEXT("Room service unavailable."));
		return;
	}

	const FString SafeRoomId = RoomId.TrimStartAndEnd();
	if (SafeRoomId.IsEmpty())
	{
		ReportBackendError(TEXT("RoomId is empty."));
		return;
	}

	PendingJoinRoomId = SafeRoomId;
	SetBackendState(EDBALobbyBackendState::Loading);

	FGameBackendResponseDelegate Callback;
	Callback.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UDBAMainLobbyWidgetController, HandleJoinRoomResponse));
	RoomService->JoinRoom(SafeRoomId, Callback);
}

void UDBAMainLobbyWidgetController::LeaveRoom()
{
	UGameBackendClientSubsystem* Backend = GetBackendSubsystem();
	UGameBackendRoomService* RoomService = Backend ? Backend->GetRoomService() : nullptr;
	if (!RoomService)
	{
		ReportBackendError(TEXT("Room service unavailable."));
		return;
	}

	if (CurrentRoomId.IsEmpty())
	{
		ResolveStateAfterBackgroundRefresh();
		return;
	}

	SetBackendState(EDBALobbyBackendState::Loading);
	FGameBackendResponseDelegate Callback;
	Callback.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UDBAMainLobbyWidgetController, HandleLeaveRoomResponse));
	RoomService->LeaveRoom(CurrentRoomId, Callback);
}

void UDBAMainLobbyWidgetController::SetReady(bool bReady)
{
	UGameBackendClientSubsystem* Backend = GetBackendSubsystem();
	UGameBackendRoomService* RoomService = Backend ? Backend->GetRoomService() : nullptr;
	if (!RoomService)
	{
		ReportBackendError(TEXT("Room service unavailable."));
		return;
	}

	if (CurrentRoomId.IsEmpty())
	{
		ReportBackendError(TEXT("Not in room."));
		return;
	}

	bPendingReadyState = bReady;
	SetBackendState(EDBALobbyBackendState::Loading);

	FGameBackendResponseDelegate Callback;
	Callback.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UDBAMainLobbyWidgetController, HandleSetReadyResponse));
	RoomService->SetReady(CurrentRoomId, bReady, Callback);
}

void UDBAMainLobbyWidgetController::StartRoom()
{
	UGameBackendClientSubsystem* Backend = GetBackendSubsystem();
	UGameBackendRoomService* RoomService = Backend ? Backend->GetRoomService() : nullptr;
	if (!RoomService)
	{
		ReportBackendError(TEXT("Room service unavailable."));
		return;
	}

	if (CurrentRoomId.IsEmpty())
	{
		ReportBackendError(TEXT("Not in room."));
		return;
	}

	SetBackendState(EDBALobbyBackendState::Loading);

	FGameBackendResponseDelegate Callback;
	Callback.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UDBAMainLobbyWidgetController, HandleStartRoomResponse));
	RoomService->StartRoom(CurrentRoomId, Callback);
}

void UDBAMainLobbyWidgetController::StartMatchmaking(const FString& Mode, const FString& RegionCode)
{
	UGameBackendClientSubsystem* Backend = GetBackendSubsystem();
	UGameBackendMatchService* MatchService = Backend ? Backend->GetMatchService() : nullptr;
	if (!MatchService)
	{
		ReportBackendError(TEXT("Match service unavailable."));
		return;
	}

	const FString SafeMode = Mode.TrimStartAndEnd().IsEmpty() ? TEXT("default") : Mode.TrimStartAndEnd();
	const FString SafeRegion = RegionCode.TrimStartAndEnd().IsEmpty() ? TEXT("local") : RegionCode.TrimStartAndEnd();
	PendingMatchMode = SafeMode;
	PendingMatchRegion = SafeRegion;

	SetBackendState(EDBALobbyBackendState::Loading);
	FGameBackendResponseDelegate Callback;
	Callback.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UDBAMainLobbyWidgetController, HandleCreateTicketResponse));
	MatchService->CreateTicket(SafeMode, SafeRegion, Callback);
}

void UDBAMainLobbyWidgetController::CancelMatchmaking()
{
	UGameBackendClientSubsystem* Backend = GetBackendSubsystem();
	UGameBackendMatchService* MatchService = Backend ? Backend->GetMatchService() : nullptr;
	if (!MatchService)
	{
		ReportBackendError(TEXT("Match service unavailable."));
		return;
	}

	StopTicketPolling();
	if (CurrentTicketId.IsEmpty())
	{
		ResolveStateAfterBackgroundRefresh();
		return;
	}

	FGameBackendResponseDelegate Callback;
	Callback.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UDBAMainLobbyWidgetController, HandleCancelTicketResponse));
	MatchService->CancelTicket(CurrentTicketId, Callback);
}

void UDBAMainLobbyWidgetController::NotifyMatchFinishedClientView()
{
	TrackTelemetry(TEXT("match_finished_client_view"), TMap<FString, FString>());
}

void UDBAMainLobbyWidgetController::HandleGetProfileResponse(bool bSuccess, const FString& ErrorMessage, const FString& DataJson)
{
	if (!bSuccess)
	{
		ReportBackendError(ErrorMessage);
		return;
	}

	bProfileLoaded = true;
	OnProfileUpdated.Broadcast(DataJson);
	UpdatePlayerSummaryFromProfile(DataJson);

	UGameBackendClientSubsystem* Backend = GetBackendSubsystem();
	UGameBackendPlayerService* PlayerService = Backend ? Backend->GetPlayerService() : nullptr;
	if (!PlayerService)
	{
		ReportBackendError(TEXT("Player service unavailable."));
		return;
	}

	FGameBackendResponseDelegate Callback;
	Callback.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UDBAMainLobbyWidgetController, HandleGetInventoryResponse));
	PlayerService->GetMyInventory(Callback);
}

void UDBAMainLobbyWidgetController::HandleGetInventoryResponse(bool bSuccess, const FString& ErrorMessage, const FString& DataJson)
{
	if (!bSuccess)
	{
		ReportBackendError(ErrorMessage);
		return;
	}

	bInventoryLoaded = true;
	OnInventoryUpdated.Broadcast(DataJson);
	UpdatePlayerSummaryFromInventory(DataJson);
	OnPlayerSummaryUpdated.Broadcast(PlayerSummary);
	ResolveStateAfterBackgroundRefresh();
}

void UDBAMainLobbyWidgetController::HandleGetRoomsResponse(bool bSuccess, const FString& ErrorMessage, const FString& DataJson)
{
	if (!bSuccess)
	{
		ReportBackendError(ErrorMessage);
		return;
	}

	OnRoomsUpdated.Broadcast(DataJson);
	ResolveStateAfterBackgroundRefresh();
}

void UDBAMainLobbyWidgetController::HandleCreateRoomResponse(bool bSuccess, const FString& ErrorMessage, const FString& DataJson)
{
	if (!bSuccess)
	{
		ReportBackendError(ErrorMessage);
		return;
	}

	const FString RoomId = ExtractStringByKeys(DataJson, { TEXT("roomId"), TEXT("room_id"), TEXT("id") });
	if (!RoomId.IsEmpty())
	{
		CurrentRoomId = RoomId;
	}
	bReadyLocalState = false;
	OnRoomsUpdated.Broadcast(DataJson);
	SetBackendState(EDBALobbyBackendState::InRoom);
}

void UDBAMainLobbyWidgetController::HandleJoinRoomResponse(bool bSuccess, const FString& ErrorMessage, const FString& DataJson)
{
	if (!bSuccess)
	{
		ReportBackendError(ErrorMessage);
		return;
	}

	if (!PendingJoinRoomId.IsEmpty())
	{
		CurrentRoomId = PendingJoinRoomId;
	}
	else
	{
		const FString RoomId = ExtractStringByKeys(DataJson, { TEXT("roomId"), TEXT("room_id"), TEXT("id") });
		if (!RoomId.IsEmpty())
		{
			CurrentRoomId = RoomId;
		}
	}
	PendingJoinRoomId.Empty();
	bReadyLocalState = false;
	OnRoomsUpdated.Broadcast(DataJson);
	SetBackendState(EDBALobbyBackendState::InRoom);
}

void UDBAMainLobbyWidgetController::HandleLeaveRoomResponse(bool bSuccess, const FString& ErrorMessage, const FString& DataJson)
{
	if (!bSuccess)
	{
		ReportBackendError(ErrorMessage);
		return;
	}

	CurrentRoomId.Empty();
	bReadyLocalState = false;
	OnRoomsUpdated.Broadcast(DataJson);
	ResolveStateAfterBackgroundRefresh();
	RefreshRoomList();
}

void UDBAMainLobbyWidgetController::HandleSetReadyResponse(bool bSuccess, const FString& ErrorMessage, const FString& DataJson)
{
	if (!bSuccess)
	{
		ReportBackendError(ErrorMessage);
		return;
	}

	bReadyLocalState = bPendingReadyState;
	OnRoomsUpdated.Broadcast(DataJson);
	SetBackendState(bReadyLocalState ? EDBALobbyBackendState::Ready : EDBALobbyBackendState::InRoom);
}

void UDBAMainLobbyWidgetController::HandleStartRoomResponse(bool bSuccess, const FString& ErrorMessage, const FString& DataJson)
{
	if (!bSuccess)
	{
		ReportBackendError(ErrorMessage);
		return;
	}

	OnRoomsUpdated.Broadcast(DataJson);
	SetBackendState(EDBALobbyBackendState::InRoom);
}

void UDBAMainLobbyWidgetController::HandleCreateTicketResponse(bool bSuccess, const FString& ErrorMessage, const FString& DataJson)
{
	if (!bSuccess)
	{
		ReportBackendError(ErrorMessage);
		return;
	}

	CurrentTicketId = ExtractStringByKeys(DataJson, { TEXT("ticketId"), TEXT("ticket_id"), TEXT("id") });
	if (CurrentTicketId.IsEmpty())
	{
		ReportBackendError(TEXT("Ticket created but ticket id is missing."));
		return;
	}

	OnMatchTicketUpdated.Broadcast(DataJson);
	SetBackendState(EDBALobbyBackendState::Matching);

	TMap<FString, FString> Props;
	Props.Add(TEXT("mode"), PendingMatchMode);
	Props.Add(TEXT("region"), PendingMatchRegion);
	TrackTelemetry(TEXT("matchmaking_started"), Props);

	StartTicketPolling();
}

void UDBAMainLobbyWidgetController::HandleGetTicketResponse(bool bSuccess, const FString& ErrorMessage, const FString& DataJson)
{
	if (!bSuccess)
	{
		ReportBackendError(ErrorMessage);
		return;
	}

	OnMatchTicketUpdated.Broadcast(DataJson);
	ResolveMatchAndConnect(DataJson);
}

void UDBAMainLobbyWidgetController::HandleCancelTicketResponse(bool bSuccess, const FString& ErrorMessage, const FString& DataJson)
{
	if (!bSuccess)
	{
		ReportBackendError(ErrorMessage);
		return;
	}

	CurrentTicketId.Empty();
	CurrentSessionId.Empty();
	OnMatchTicketUpdated.Broadcast(DataJson);
	ResolveStateAfterBackgroundRefresh();
}

void UDBAMainLobbyWidgetController::HandleGetSessionResponse(bool bSuccess, const FString& ErrorMessage, const FString& DataJson)
{
	if (!bSuccess)
	{
		ReportBackendError(ErrorMessage);
		return;
	}

	UGameBackendClientSubsystem* Backend = GetBackendSubsystem();
	UGameBackendSessionService* SessionService = Backend ? Backend->GetSessionService() : nullptr;
	if (!SessionService)
	{
		ReportBackendError(TEXT("Session service unavailable."));
		return;
	}

	SetBackendState(EDBALobbyBackendState::Connecting);
	{
		TMap<FString, FString> Props;
		Props.Add(TEXT("sessionId"), CurrentSessionId);
		TrackTelemetry(TEXT("connect_server_started"), Props);
	}

	FGameBackendResponseDelegate Callback;
	Callback.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UDBAMainLobbyWidgetController, HandleGetConnectionResponse));
	SessionService->GetConnection(CurrentSessionId, Callback);
}

void UDBAMainLobbyWidgetController::HandleGetConnectionResponse(bool bSuccess, const FString& ErrorMessage, const FString& DataJson)
{
	if (!bSuccess)
	{
		TMap<FString, FString> Props;
		Props.Add(TEXT("reason"), ErrorMessage.Left(256));
		TrackTelemetry(TEXT("connect_server_failed"), Props);
		ReportBackendError(ErrorMessage);
		return;
	}

	UGameBackendClientSubsystem* Backend = GetBackendSubsystem();
	UGameBackendSessionService* SessionService = Backend ? Backend->GetSessionService() : nullptr;
	if (!SessionService)
	{
		ReportBackendError(TEXT("Session service unavailable."));
		return;
	}

	FGameBackendResponseDelegate Callback;
	Callback.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UDBAMainLobbyWidgetController, HandleConnectToServerResponse));
	SessionService->ConnectToDedicatedServer(CurrentSessionId, DataJson, Callback);
}

void UDBAMainLobbyWidgetController::HandleConnectToServerResponse(bool bSuccess, const FString& ErrorMessage, const FString& DataJson)
{
	if (!bSuccess)
	{
		TMap<FString, FString> Props;
		Props.Add(TEXT("reason"), ErrorMessage.Left(256));
		TrackTelemetry(TEXT("connect_server_failed"), Props);
		ReportBackendError(ErrorMessage);
		return;
	}

	TMap<FString, FString> SuccessProps;
	SuccessProps.Add(TEXT("sessionId"), CurrentSessionId);
	TrackTelemetry(TEXT("connect_server_succeeded"), SuccessProps);
	SetBackendState(EDBALobbyBackendState::Connecting);

	// DataJson for this callback is the travel url when connection is successful.
	OnMatchTicketUpdated.Broadcast(DataJson);
}

void UDBAMainLobbyWidgetController::UpdatePlayerSummaryFromProfile(const FString& ProfileJson)
{
	TSharedPtr<FJsonObject> Root;
	if (!TryParseJsonObject(ProfileJson, Root))
	{
		return;
	}

	const FString DisplayName = ReadStringByKeysDeep(Root, { TEXT("displayName"), TEXT("playerName"), TEXT("nickname"), TEXT("name") });
	if (!DisplayName.IsEmpty())
	{
		PlayerSummary.DisplayName = DisplayName;
	}

	const FString PlayerId = ReadStringByKeysDeep(Root, { TEXT("playerId"), TEXT("player_id"), TEXT("uid") });
	if (!PlayerId.IsEmpty())
	{
		PlayerSummary.PlayerId = PlayerId;
	}

	const int32 Level = ReadIntByKeysDeep(Root, { TEXT("level"), TEXT("playerLevel"), TEXT("lv"), TEXT("lvl") });
	if (Level > 0)
	{
		PlayerSummary.Level = Level;
	}

	const int32 Experience = ReadIntByKeysDeep(Root, { TEXT("experience"), TEXT("exp"), TEXT("xp"), TEXT("currentExp") });
	PlayerSummary.Experience = FMath::Max(0, Experience);
}

void UDBAMainLobbyWidgetController::UpdatePlayerSummaryFromInventory(const FString& InventoryJson)
{
	TSharedPtr<FJsonObject> Root;
	if (!TryParseJsonObject(InventoryJson, Root))
	{
		return;
	}

	const int32 Gold = ReadIntByKeysDeep(Root, { TEXT("gold"), TEXT("coins"), TEXT("coin"), TEXT("currencyGold") });
	PlayerSummary.Gold = FMath::Max(0, Gold);

	const int32 Tickets = ReadIntByKeysDeep(Root, { TEXT("tickets"), TEXT("ticket"), TEXT("matchTickets"), TEXT("coupon") });
	PlayerSummary.Tickets = FMath::Max(0, Tickets);
}

void UDBAMainLobbyWidgetController::ResolveStateAfterBackgroundRefresh()
{
	if (BackendState == EDBALobbyBackendState::Matching || BackendState == EDBALobbyBackendState::MatchFound || BackendState == EDBALobbyBackendState::Connecting)
	{
		return;
	}

	if (!CurrentTicketId.IsEmpty())
	{
		SetBackendState(EDBALobbyBackendState::Matching);
		return;
	}

	if (!CurrentRoomId.IsEmpty())
	{
		SetBackendState(bReadyLocalState ? EDBALobbyBackendState::Ready : EDBALobbyBackendState::InRoom);
		return;
	}

	SetBackendState(EDBALobbyBackendState::Idle);
}

void UDBAMainLobbyWidgetController::SetBackendState(EDBALobbyBackendState NewState)
{
	if (BackendState == NewState)
	{
		return;
	}

	BackendState = NewState;
	OnBackendStateChanged.Broadcast(NewState);
	UE_LOG(LogDBAUI, Log, TEXT("[MainLobby] Backend state switched to %d"), static_cast<int32>(NewState));
}

void UDBAMainLobbyWidgetController::ReportBackendError(const FString& ErrorMessage)
{
	const FString SafeError = ErrorMessage.IsEmpty() ? TEXT("Backend request failed.") : ErrorMessage;
	SetBackendState(EDBALobbyBackendState::Error);
	OnBackendError.Broadcast(SafeError);
	UE_LOG(LogDBAUI, Warning, TEXT("[MainLobby] Backend error: %s"), *SafeError);
}

void UDBAMainLobbyWidgetController::StartTicketPolling()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		ReportBackendError(TEXT("World context is invalid, cannot start ticket polling."));
		return;
	}

	FTimerManager& TimerManager = World->GetTimerManager();
	TimerManager.ClearTimer(TicketPollingTimerHandle);
	TimerManager.SetTimer(TicketPollingTimerHandle, this, &UDBAMainLobbyWidgetController::PollTicketOnce, 2.0f, true, 2.0f);
}

void UDBAMainLobbyWidgetController::StopTicketPolling()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TicketPollingTimerHandle);
	}
}

void UDBAMainLobbyWidgetController::PollTicketOnce()
{
	UGameBackendClientSubsystem* Backend = GetBackendSubsystem();
	UGameBackendMatchService* MatchService = Backend ? Backend->GetMatchService() : nullptr;
	if (!MatchService || CurrentTicketId.IsEmpty())
	{
		StopTicketPolling();
		return;
	}

	FGameBackendResponseDelegate Callback;
	Callback.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UDBAMainLobbyWidgetController, HandleGetTicketResponse));
	MatchService->GetTicket(CurrentTicketId, Callback);
}

void UDBAMainLobbyWidgetController::ResolveMatchAndConnect(const FString& TicketDataJson)
{
	FString SessionId;
	if (!IsTicketMatched(TicketDataJson, SessionId))
	{
		return;
	}

	StopTicketPolling();
	CurrentSessionId = SessionId;
	SetBackendState(EDBALobbyBackendState::MatchFound);
	TMap<FString, FString> Props;
	Props.Add(TEXT("sessionId"), SessionId);
	TrackTelemetry(TEXT("matchmaking_found"), Props);

	UGameBackendClientSubsystem* Backend = GetBackendSubsystem();
	UGameBackendSessionService* SessionService = Backend ? Backend->GetSessionService() : nullptr;
	if (!SessionService)
	{
		ReportBackendError(TEXT("Session service unavailable."));
		return;
	}

	FGameBackendResponseDelegate Callback;
	Callback.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UDBAMainLobbyWidgetController, HandleGetSessionResponse));
	SessionService->GetSession(SessionId, Callback);
}

void UDBAMainLobbyWidgetController::TrackTelemetry(const FString& EventName, const TMap<FString, FString>& Properties) const
{
	if (UGameBackendClientSubsystem* Backend = GetBackendSubsystem())
	{
		if (Backend->GetTelemetryService())
		{
			Backend->GetTelemetryService()->TrackEvent(EventName, Properties);
		}
	}
}

FString UDBAMainLobbyWidgetController::ExtractStringByKeys(const FString& Json, const TArray<FString>& Keys) const
{
	TSharedPtr<FJsonObject> Root;
	if (!TryParseJsonObject(Json, Root))
	{
		return FString();
	}

	return ReadStringByKeysDeep(Root, Keys);
}

bool UDBAMainLobbyWidgetController::IsTicketMatched(const FString& TicketDataJson, FString& OutSessionId) const
{
	TSharedPtr<FJsonObject> Root;
	if (!TryParseJsonObject(TicketDataJson, Root))
	{
		return false;
	}

	const FString Status = ReadStringByKeysDeep(Root, { TEXT("status"), TEXT("state") });
	OutSessionId = ReadStringByKeysDeep(Root, { TEXT("sessionId"), TEXT("session_id") });

	return Status.Equals(TEXT("matched"), ESearchCase::IgnoreCase)
		|| Status.Equals(TEXT("found"), ESearchCase::IgnoreCase)
		|| !OutSessionId.IsEmpty();
}

UGameBackendClientSubsystem* UDBAMainLobbyWidgetController::GetBackendSubsystem() const
{
	return GetWorld() && GetWorld()->GetGameInstance()
		? GetWorld()->GetGameInstance()->GetSubsystem<UGameBackendClientSubsystem>()
		: nullptr;
}
