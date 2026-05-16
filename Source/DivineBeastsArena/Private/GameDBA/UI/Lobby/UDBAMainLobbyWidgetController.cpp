// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Lobby/UDBAMainLobbyWidgetController.h"

#include "Dom/JsonObject.h"
#include "GameBackendClientSubsystem.h"
#include "GameBackendHttpClient.h"
#include "GameBackendSessionService.h"
#include "GameBackendTelemetryService.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "TimerManager.h"

namespace
{
	FString ReadStringByKeys(const TSharedPtr<FJsonObject>& Obj, const TArray<FString>& Keys)
	{
		if (!Obj.IsValid())
		{
			return FString();
		}

		for (const FString& Key : Keys)
		{
			FString Value;
			if (Obj->TryGetStringField(Key, Value))
			{
				return Value;
			}
		}

		return FString();
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
	UE_LOG(LogDBAUI, Log, TEXT("[MainLobby] 鍒囨崲闃佃惀涓婚璇锋眰: %d"), FiveCamp);
}

void UDBAMainLobbyWidgetController::RequestNavigateToNewbieVillage()
{
	UE_LOG(LogDBAUI, Log, TEXT("[MainLobby] \u8bf7\u6c42\u8fdb\u5165\u65b0\u624b\u6751\u3002"));
}

void UDBAMainLobbyWidgetController::RequestNavigateToPractice()
{
	UE_LOG(LogDBAUI, Log, TEXT("[MainLobby] \u8bf7\u6c42\u8fdb\u5165\u7ec3\u4e60\u6a21\u5f0f\u3002"));
}

void UDBAMainLobbyWidgetController::RequestExitGame()
{
	UE_LOG(LogDBAUI, Log, TEXT("[MainLobby] \u8bf7\u6c42\u9000\u51fa\u6e38\u620f\u3002"));
}

void UDBAMainLobbyWidgetController::InitializeBackendLobby()
{
	SetBackendState(EDBALobbyBackendState::Idle);
	RefreshPlayerData();
	RefreshRoomList();
}

void UDBAMainLobbyWidgetController::RefreshPlayerData()
{
	UGameBackendClientSubsystem* Backend = GetBackendSubsystem();
	FGameBackendHttpClient* Http = Backend ? Backend->GetHttpClient() : nullptr;
	if (!Http)
	{
		ReportBackendError(TEXT("\u540e\u7aef HTTP \u5ba2\u6237\u7aef\u4e0d\u53ef\u7528\u3002"));
		return;
	}

	SetBackendState(EDBALobbyBackendState::Loading);
	Http->Get(TEXT("/api/players/me/profile"), [this, Http](const FGameBackendHttpResult& ProfileResult)
	{
		const bool bProfileOk = ProfileResult.bHttpRequestOk && ProfileResult.HttpStatus >= 200 && ProfileResult.HttpStatus < 300;
		if (!bProfileOk)
		{
			ReportBackendError(ProfileResult.Message);
			return;
		}
		OnProfileUpdated.Broadcast(ProfileResult.DataJson);

		Http->Get(TEXT("/api/players/me/inventory"), [this](const FGameBackendHttpResult& InventoryResult)
		{
			const bool bInventoryOk = InventoryResult.bHttpRequestOk && InventoryResult.HttpStatus >= 200 && InventoryResult.HttpStatus < 300;
			if (!bInventoryOk)
			{
				ReportBackendError(InventoryResult.Message);
				return;
			}

			OnInventoryUpdated.Broadcast(InventoryResult.DataJson);
			SetBackendState(EDBALobbyBackendState::Idle);
		});
	});
}

void UDBAMainLobbyWidgetController::RefreshRoomList()
{
	UGameBackendClientSubsystem* Backend = GetBackendSubsystem();
	FGameBackendHttpClient* Http = Backend ? Backend->GetHttpClient() : nullptr;
	if (!Http)
	{
		ReportBackendError(TEXT("\u540e\u7aef HTTP \u5ba2\u6237\u7aef\u4e0d\u53ef\u7528\u3002"));
		return;
	}

	SetBackendState(EDBALobbyBackendState::Loading);
	Http->Get(TEXT("/api/rooms"), [this](const FGameBackendHttpResult& Result)
	{
		const bool bSuccess = Result.bHttpRequestOk && Result.HttpStatus >= 200 && Result.HttpStatus < 300;
		if (!bSuccess)
		{
			ReportBackendError(Result.Message);
			return;
		}
		OnRoomsUpdated.Broadcast(Result.DataJson);
		SetBackendState(EDBALobbyBackendState::Idle);
	});
}

void UDBAMainLobbyWidgetController::CreateRoom()
{
	UGameBackendClientSubsystem* Backend = GetBackendSubsystem();
	FGameBackendHttpClient* Http = Backend ? Backend->GetHttpClient() : nullptr;
	if (!Http)
	{
		ReportBackendError(TEXT("\u540e\u7aef HTTP \u5ba2\u6237\u7aef\u4e0d\u53ef\u7528\u3002"));
		return;
	}

	SetBackendState(EDBALobbyBackendState::Loading);

	const TSharedRef<FJsonObject> Request = MakeShared<FJsonObject>();
	Request->SetStringField(TEXT("mode"), TEXT("default"));
	Request->SetStringField(TEXT("region"), TEXT("local"));
	Request->SetNumberField(TEXT("maxPlayers"), 10);
	Request->SetBoolField(TEXT("private"), false);
	FString Body;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
	FJsonSerializer::Serialize(Request, Writer);

	Http->Post(TEXT("/api/rooms"), Body, [this](const FGameBackendHttpResult& Result)
	{
		const bool bSuccess = Result.bHttpRequestOk && Result.HttpStatus >= 200 && Result.HttpStatus < 300;
		if (!bSuccess)
		{
			ReportBackendError(Result.Message);
			return;
		}
		CurrentRoomId = ExtractStringByKeys(Result.DataJson, { TEXT("roomId"), TEXT("room_id"), TEXT("id") });
		OnRoomsUpdated.Broadcast(Result.DataJson);
		SetBackendState(EDBALobbyBackendState::InRoom);
	});
}

void UDBAMainLobbyWidgetController::JoinRoom(const FString& RoomId)
{
	UGameBackendClientSubsystem* Backend = GetBackendSubsystem();
	FGameBackendHttpClient* Http = Backend ? Backend->GetHttpClient() : nullptr;
	if (!Http)
	{
		ReportBackendError(TEXT("\u540e\u7aef HTTP \u5ba2\u6237\u7aef\u4e0d\u53ef\u7528\u3002"));
		return;
	}

	const FString SafeRoomId = RoomId.TrimStartAndEnd();
	if (SafeRoomId.IsEmpty())
	{
		ReportBackendError(TEXT("\u623f\u95f4 ID \u4e0d\u80fd\u4e3a\u7a7a\u3002"));
		return;
	}

	SetBackendState(EDBALobbyBackendState::Loading);
	Http->Post(FString::Printf(TEXT("/api/rooms/%s/join"), *SafeRoomId), TEXT("{}"), [this, SafeRoomId](const FGameBackendHttpResult& Result)
	{
		const bool bSuccess = Result.bHttpRequestOk && Result.HttpStatus >= 200 && Result.HttpStatus < 300;
		if (!bSuccess)
		{
			ReportBackendError(Result.Message);
			return;
		}
		CurrentRoomId = SafeRoomId;
		OnRoomsUpdated.Broadcast(Result.DataJson);
		SetBackendState(EDBALobbyBackendState::InRoom);
	});
}

void UDBAMainLobbyWidgetController::LeaveRoom()
{
	UGameBackendClientSubsystem* Backend = GetBackendSubsystem();
	FGameBackendHttpClient* Http = Backend ? Backend->GetHttpClient() : nullptr;
	if (!Http)
	{
		ReportBackendError(TEXT("\u540e\u7aef HTTP \u5ba2\u6237\u7aef\u4e0d\u53ef\u7528\u3002"));
		return;
	}

	if (CurrentRoomId.IsEmpty())
	{
		SetBackendState(EDBALobbyBackendState::Idle);
		return;
	}

	SetBackendState(EDBALobbyBackendState::Loading);
	Http->Post(FString::Printf(TEXT("/api/rooms/%s/leave"), *CurrentRoomId), TEXT("{}"), [this](const FGameBackendHttpResult& Result)
	{
		const bool bSuccess = Result.bHttpRequestOk && Result.HttpStatus >= 200 && Result.HttpStatus < 300;
		if (!bSuccess)
		{
			ReportBackendError(Result.Message);
			return;
		}

		CurrentRoomId.Empty();
		SetBackendState(EDBALobbyBackendState::Idle);
		RefreshRoomList();
	});
}

void UDBAMainLobbyWidgetController::SetReady(bool bReady)
{
	UGameBackendClientSubsystem* Backend = GetBackendSubsystem();
	FGameBackendHttpClient* Http = Backend ? Backend->GetHttpClient() : nullptr;
	if (!Http)
	{
		ReportBackendError(TEXT("\u540e\u7aef HTTP \u5ba2\u6237\u7aef\u4e0d\u53ef\u7528\u3002"));
		return;
	}

	if (CurrentRoomId.IsEmpty())
	{
		ReportBackendError(TEXT("\u5f53\u524d\u672a\u52a0\u5165\u623f\u95f4\u3002"));
		return;
	}

	SetBackendState(EDBALobbyBackendState::Loading);

	const TSharedRef<FJsonObject> Request = MakeShared<FJsonObject>();
	Request->SetBoolField(TEXT("isReady"), bReady);
	FString Body;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
	FJsonSerializer::Serialize(Request, Writer);

	Http->Post(FString::Printf(TEXT("/api/rooms/%s/ready"), *CurrentRoomId), Body, [this, bReady](const FGameBackendHttpResult& Result)
	{
		const bool bSuccess = Result.bHttpRequestOk && Result.HttpStatus >= 200 && Result.HttpStatus < 300;
		if (!bSuccess)
		{
			ReportBackendError(Result.Message);
			return;
		}

		OnRoomsUpdated.Broadcast(Result.DataJson);
		SetBackendState(bReady ? EDBALobbyBackendState::Ready : EDBALobbyBackendState::InRoom);
	});
}

void UDBAMainLobbyWidgetController::StartRoom()
{
	UGameBackendClientSubsystem* Backend = GetBackendSubsystem();
	FGameBackendHttpClient* Http = Backend ? Backend->GetHttpClient() : nullptr;
	if (!Http)
	{
		ReportBackendError(TEXT("\u540e\u7aef HTTP \u5ba2\u6237\u7aef\u4e0d\u53ef\u7528\u3002"));
		return;
	}

	if (CurrentRoomId.IsEmpty())
	{
		ReportBackendError(TEXT("\u5f53\u524d\u672a\u52a0\u5165\u623f\u95f4\u3002"));
		return;
	}

	SetBackendState(EDBALobbyBackendState::Loading);
	Http->Post(FString::Printf(TEXT("/api/rooms/%s/start"), *CurrentRoomId), TEXT("{}"), [this](const FGameBackendHttpResult& Result)
	{
		const bool bSuccess = Result.bHttpRequestOk && Result.HttpStatus >= 200 && Result.HttpStatus < 300;
		if (!bSuccess)
		{
			ReportBackendError(Result.Message);
			return;
		}

		OnRoomsUpdated.Broadcast(Result.DataJson);
		SetBackendState(EDBALobbyBackendState::InRoom);
	});
}

void UDBAMainLobbyWidgetController::StartMatchmaking(const FString& Mode, const FString& RegionCode)
{
	UGameBackendClientSubsystem* Backend = GetBackendSubsystem();
	FGameBackendHttpClient* Http = Backend ? Backend->GetHttpClient() : nullptr;
	if (!Http)
	{
		ReportBackendError(TEXT("\u540e\u7aef HTTP \u5ba2\u6237\u7aef\u4e0d\u53ef\u7528\u3002"));
		return;
	}

	SetBackendState(EDBALobbyBackendState::Loading);

	const TSharedRef<FJsonObject> Request = MakeShared<FJsonObject>();
	Request->SetStringField(TEXT("mode"), Mode);
	Request->SetStringField(TEXT("region"), RegionCode);
	FString Body;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
	FJsonSerializer::Serialize(Request, Writer);

	Http->Post(TEXT("/api/matchmaking/tickets"), Body, [this, Mode, RegionCode](const FGameBackendHttpResult& Result)
	{
		const bool bSuccess = Result.bHttpRequestOk && Result.HttpStatus >= 200 && Result.HttpStatus < 300;
		if (!bSuccess)
		{
			ReportBackendError(Result.Message);
			return;
		}

		CurrentTicketId = ExtractStringByKeys(Result.DataJson, { TEXT("ticketId"), TEXT("ticket_id"), TEXT("id") });
		if (CurrentTicketId.IsEmpty())
		{
			ReportBackendError(TEXT("\u521b\u5efa\u5339\u914d\u7968\u636e\u6210\u529f\uff0c\u4f46\u672a\u8fd4\u56de TicketId\u3002"));
			return;
		}

		OnMatchTicketUpdated.Broadcast(Result.DataJson);
		SetBackendState(EDBALobbyBackendState::Matching);
		TMap<FString, FString> Props;
		Props.Add(TEXT("mode"), Mode);
		Props.Add(TEXT("region"), RegionCode);
		TrackTelemetry(TEXT("matchmaking_started"), Props);
		StartTicketPolling();
	});
}

void UDBAMainLobbyWidgetController::CancelMatchmaking()
{
	UGameBackendClientSubsystem* Backend = GetBackendSubsystem();
	FGameBackendHttpClient* Http = Backend ? Backend->GetHttpClient() : nullptr;
	if (!Http)
	{
		ReportBackendError(TEXT("\u540e\u7aef HTTP \u5ba2\u6237\u7aef\u4e0d\u53ef\u7528\u3002"));
		return;
	}

	StopTicketPolling();
	if (CurrentTicketId.IsEmpty())
	{
		SetBackendState(EDBALobbyBackendState::Idle);
		return;
	}

	Http->Delete(FString::Printf(TEXT("/api/matchmaking/tickets/%s"), *CurrentTicketId), [this](const FGameBackendHttpResult& Result)
	{
		const bool bSuccess = Result.bHttpRequestOk && Result.HttpStatus >= 200 && Result.HttpStatus < 300;
		if (!bSuccess)
		{
			ReportBackendError(Result.Message);
			return;
		}

		CurrentTicketId.Empty();
		CurrentSessionId.Empty();
		SetBackendState(EDBALobbyBackendState::Idle);
	});
}

void UDBAMainLobbyWidgetController::NotifyMatchFinishedClientView()
{
	TrackTelemetry(TEXT("match_finished_client_view"), TMap<FString, FString>());
}

void UDBAMainLobbyWidgetController::SetBackendState(EDBALobbyBackendState NewState)
{
	if (BackendState == NewState)
	{
		return;
	}
	BackendState = NewState;
	OnBackendStateChanged.Broadcast(NewState);
	UE_LOG(LogDBAUI, Log, TEXT("[MainLobby] 鍚庣鐘舵€佸垏鎹? %d"), static_cast<int32>(NewState));
}

void UDBAMainLobbyWidgetController::ReportBackendError(const FString& ErrorMessage)
{
	const FString SafeError = ErrorMessage.IsEmpty() ? TEXT("\u540e\u7aef\u8bf7\u6c42\u5931\u8d25\u3002") : ErrorMessage;
	SetBackendState(EDBALobbyBackendState::Error);
	OnBackendError.Broadcast(SafeError);
	UE_LOG(LogDBAUI, Warning, TEXT("[MainLobby] 鍚庣閿欒: %s"), *SafeError);
}

void UDBAMainLobbyWidgetController::StartTicketPolling()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		ReportBackendError(TEXT("\u4e16\u754c\u4e0a\u4e0b\u6587\u4e0d\u53ef\u7528\uff0c\u65e0\u6cd5\u8f6e\u8be2\u5339\u914d\u7968\u636e\u3002"));
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
	FGameBackendHttpClient* Http = Backend ? Backend->GetHttpClient() : nullptr;
	if (!Http || CurrentTicketId.IsEmpty())
	{
		StopTicketPolling();
		return;
	}

	Http->Get(FString::Printf(TEXT("/api/matchmaking/tickets/%s"), *CurrentTicketId), [this](const FGameBackendHttpResult& Result)
	{
		const bool bSuccess = Result.bHttpRequestOk && Result.HttpStatus >= 200 && Result.HttpStatus < 300;
		if (!bSuccess)
		{
			ReportBackendError(Result.Message);
			return;
		}

		OnMatchTicketUpdated.Broadcast(Result.DataJson);
		ResolveMatchAndConnect(Result.DataJson);
	});
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
	{
		TMap<FString, FString> Props;
		Props.Add(TEXT("sessionId"), SessionId);
		TrackTelemetry(TEXT("matchmaking_found"), Props);
	}

	UGameBackendClientSubsystem* Backend = GetBackendSubsystem();
	FGameBackendHttpClient* Http = Backend ? Backend->GetHttpClient() : nullptr;
	if (!Http || !Backend || !Backend->GetSessionService())
	{
		ReportBackendError(TEXT("\u540e\u7aef\u4f1a\u8bdd\u670d\u52a1\u4e0d\u53ef\u7528\u3002"));
		return;
	}

	Http->Get(FString::Printf(TEXT("/api/sessions/%s"), *SessionId), [this, Backend, Http, SessionId](const FGameBackendHttpResult& SessionResult)
	{
		const bool bSessionOk = SessionResult.bHttpRequestOk && SessionResult.HttpStatus >= 200 && SessionResult.HttpStatus < 300;
		if (!bSessionOk)
		{
			ReportBackendError(SessionResult.Message);
			return;
		}

		SetBackendState(EDBALobbyBackendState::Connecting);
		{
			TMap<FString, FString> Props;
			Props.Add(TEXT("sessionId"), SessionId);
			TrackTelemetry(TEXT("connect_server_started"), Props);
		}

		Http->Get(FString::Printf(TEXT("/api/sessions/%s/connection"), *SessionId), [this, Backend, SessionId](const FGameBackendHttpResult& ConnectionResult)
		{
			const bool bConnectionOk = ConnectionResult.bHttpRequestOk && ConnectionResult.HttpStatus >= 200 && ConnectionResult.HttpStatus < 300;
			if (!bConnectionOk)
			{
				TMap<FString, FString> Props;
				Props.Add(TEXT("reason"), ConnectionResult.Message.Left(256));
				TrackTelemetry(TEXT("connect_server_failed"), Props);
				ReportBackendError(ConnectionResult.Message);
				return;
			}

			FGameBackendResponseDelegate Callback;
			Backend->GetSessionService()->ConnectToDedicatedServer(SessionId, ConnectionResult.DataJson, Callback);
		});
	});
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
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		return FString();
	}

	FString Value = ReadStringByKeys(Root, Keys);
	if (!Value.IsEmpty())
	{
		return Value;
	}

	if (Root->HasTypedField<EJson::Object>(TEXT("ticket")))
	{
		const TSharedPtr<FJsonObject> TicketObj = Root->GetObjectField(TEXT("ticket"));
		Value = ReadStringByKeys(TicketObj, Keys);
		if (!Value.IsEmpty())
		{
			return Value;
		}
	}

	return FString();
}

bool UDBAMainLobbyWidgetController::IsTicketMatched(const FString& TicketDataJson, FString& OutSessionId) const
{
	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(TicketDataJson);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		return false;
	}

	const FString Status = ReadStringByKeys(Root, { TEXT("status"), TEXT("state") });
	OutSessionId = ReadStringByKeys(Root, { TEXT("sessionId"), TEXT("session_id") });
	if (OutSessionId.IsEmpty() && Root->HasTypedField<EJson::Object>(TEXT("ticket")))
	{
		const TSharedPtr<FJsonObject> TicketObj = Root->GetObjectField(TEXT("ticket"));
		OutSessionId = ReadStringByKeys(TicketObj, { TEXT("sessionId"), TEXT("session_id") });
	}

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
