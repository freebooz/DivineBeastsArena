// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameBackendRoomService.h"

#include "GameBackendClientSubsystem.h"
#include "GameBackendHttpClient.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace
{
	void ExecuteResponse(const FGameBackendResponseDelegate& Callback, const FGameBackendHttpResult& Result)
	{
		const bool bSuccess = Result.IsSuccessful();
		const FString ErrorMessage = bSuccess ? FString() : (Result.Message.IsEmpty() ? TEXT("Request failed.") : Result.Message);
		Callback.ExecuteIfBound(bSuccess, ErrorMessage, Result.DataJson);
	}
}

void UGameBackendRoomService::Initialize(UGameBackendClientSubsystem* InSubsystem, FGameBackendHttpClient* InHttpClient)
{
	Subsystem = InSubsystem;
	HttpClient = InHttpClient;
}

void UGameBackendRoomService::CreateRoom(const FGameBackendRoomCreateRequest& Request, const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("Room service unavailable."), TEXT("{}"));
		return;
	}

	const TSharedRef<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField(TEXT("mode"), Request.Mode);
	Json->SetStringField(TEXT("region"), Request.Region);
	Json->SetNumberField(TEXT("maxPlayers"), Request.MaxPlayers);
	Json->SetBoolField(TEXT("private"), Request.bPrivate);

	FString Body;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
	FJsonSerializer::Serialize(Json, Writer);

	HttpClient->Post(TEXT("/api/rooms"), Body, [Callback](const FGameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UGameBackendRoomService::GetRooms(const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("Room service unavailable."), TEXT("{}"));
		return;
	}

	HttpClient->Get(TEXT("/api/rooms"), [Callback](const FGameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UGameBackendRoomService::GetRoomDetail(const FString& RoomId, const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("Room service unavailable."), TEXT("{}"));
		return;
	}

	HttpClient->Get(FString::Printf(TEXT("/api/rooms/%s"), *RoomId), [Callback](const FGameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UGameBackendRoomService::JoinRoom(const FString& RoomId, const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("Room service unavailable."), TEXT("{}"));
		return;
	}

	HttpClient->Post(FString::Printf(TEXT("/api/rooms/%s/join"), *RoomId), TEXT("{}"), [Callback](const FGameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UGameBackendRoomService::LeaveRoom(const FString& RoomId, const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("Room service unavailable."), TEXT("{}"));
		return;
	}

	HttpClient->Post(FString::Printf(TEXT("/api/rooms/%s/leave"), *RoomId), TEXT("{}"), [Callback](const FGameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UGameBackendRoomService::SetReady(const FString& RoomId, bool bReady, const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("Room service unavailable."), TEXT("{}"));
		return;
	}

	const TSharedRef<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetBoolField(TEXT("ready"), bReady);

	FString Body;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
	FJsonSerializer::Serialize(Json, Writer);

	HttpClient->Post(FString::Printf(TEXT("/api/rooms/%s/ready"), *RoomId), Body, [Callback](const FGameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UGameBackendRoomService::StartRoom(const FString& RoomId, const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("Room service unavailable."), TEXT("{}"));
		return;
	}

	HttpClient->Post(FString::Printf(TEXT("/api/rooms/%s/start"), *RoomId), TEXT("{}"), [Callback](const FGameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UGameBackendRoomService::KickPlayer(const FString& RoomId, const FString& PlayerId, const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("Room service unavailable."), TEXT("{}"));
		return;
	}

	const TSharedRef<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField(TEXT("playerId"), PlayerId);

	FString Body;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
	FJsonSerializer::Serialize(Json, Writer);

	HttpClient->Post(FString::Printf(TEXT("/api/rooms/%s/kick"), *RoomId), Body, [Callback](const FGameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UGameBackendRoomService::TransferOwner(const FString& RoomId, const FString& PlayerId, const FGameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("Room service unavailable."), TEXT("{}"));
		return;
	}

	const TSharedRef<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField(TEXT("playerId"), PlayerId);

	FString Body;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
	FJsonSerializer::Serialize(Json, Writer);

	HttpClient->Post(FString::Printf(TEXT("/api/rooms/%s/transfer-owner"), *RoomId), Body, [Callback](const FGameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}
