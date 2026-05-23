// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
涓枃闃呰璇存槑锛?- 鎵€灞炲簲鐢細DBA_GameClient Unreal Engine 瀹㈡埛绔?/ Dedicated Server銆?- 鏂囦欢鑱岃矗锛氬疄鐜?UE Dedicated Server 鍒?DBA_GameBackend Runtime API 鐨?HTTP 涓婃姤銆?- 闃呰閲嶇偣锛氭墍鏈?Runtime 璇锋眰閮藉甫 serverId銆乻essionId銆乺untimeToken锛屽悗绔彧淇濆瓨 token hash 骞舵牎楠岀粦瀹氬叧绯汇€?- 淇敼鎻愮ず锛氫繚鎸佹帴鍙ｆ棤瀹㈡埛绔?JWT 渚濊禆锛孯untime Token 鏄湇鍔″櫒涓撶敤鍑瘉锛屼笉鑳芥毚闇茬粰鏅€氬鎴风銆?*/

#include "GameBackendRuntimeService.h"

#include "GameBackendClientSubsystem.h"
#include "GameBackendHttpClient.h"
#include "Dom/JsonObject.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace
{
	FString ReadCommandLineValue(const TCHAR* Key)
	{
		FString Value;
		if (FParse::Value(FCommandLine::Get(), Key, Value))
		{
			Value.TrimStartAndEndInline();
		}
		return Value;
	}
}

void UDBA_GameBackendRuntimeService::Initialize(UDBA_GameBackendClientSubsystem* InSubsystem, FDBA_GameBackendHttpClient* InHttpClient)
{
	Subsystem = InSubsystem;
	HttpClient = InHttpClient;
}

bool UDBA_GameBackendRuntimeService::ConfigureFromCommandLine()
{
	SessionId = ReadCommandLineValue(TEXT("sessionId="));
	if (SessionId.IsEmpty())
	{
		SessionId = ReadCommandLineValue(TEXT("SessionId="));
	}

	ServerId = ReadCommandLineValue(TEXT("serverId="));
	if (ServerId.IsEmpty())
	{
		ServerId = ReadCommandLineValue(TEXT("ServerId="));
	}

	RuntimeToken = ReadCommandLineValue(TEXT("runtimeToken="));
	if (RuntimeToken.IsEmpty())
	{
		RuntimeToken = ReadCommandLineValue(TEXT("RuntimeToken="));
	}

	UE_LOG(LogDBA_GameBackendClient, Log, TEXT("Runtime 鍙傛暟璇诲彇瀹屾垚銆係essionId=%s ServerId=%s 宸查厤缃甌oken=%s"),
		*SessionId,
		*ServerId,
		RuntimeToken.IsEmpty() ? TEXT("false") : TEXT("true"));

	return IsConfigured();
}

bool UDBA_GameBackendRuntimeService::IsConfigured() const
{
	return !SessionId.IsEmpty() && !ServerId.IsEmpty() && !RuntimeToken.IsEmpty();
}

void UDBA_GameBackendRuntimeService::RegisterServer(const FDBA_GameBackendResponseDelegate& Callback)
{
	PostRuntime(TEXT("/runtime/servers/register"), BuildRuntimePayload([](TSharedRef<FJsonObject>) {}), Callback);
}

void UDBA_GameBackendRuntimeService::MarkReady(const FDBA_GameBackendResponseDelegate& Callback)
{
	PostRuntime(TEXT("/runtime/servers/ready"), BuildRuntimePayload([](TSharedRef<FJsonObject>) {}), Callback);
}

void UDBA_GameBackendRuntimeService::SendHeartbeat(const FDBA_GameBackendResponseDelegate& Callback)
{
	PostRuntime(TEXT("/runtime/servers/heartbeat"), BuildRuntimePayload([](TSharedRef<FJsonObject>) {}), Callback);
}

void UDBA_GameBackendRuntimeService::NotifyPlayerJoined(
	const FString& PlayerId,
	const FString& PlayerSessionToken,
	const FString& Team,
	int32 SlotIndex,
	const FDBA_GameBackendResponseDelegate& Callback)
{
	const FString Body = BuildRuntimePayload([&PlayerId, &PlayerSessionToken, &Team, SlotIndex](TSharedRef<FJsonObject> Json)
	{
		Json->SetStringField(TEXT("playerId"), PlayerId);
		Json->SetStringField(TEXT("playerSessionToken"), PlayerSessionToken);
		Json->SetStringField(TEXT("team"), Team);
		Json->SetNumberField(TEXT("slotIndex"), SlotIndex);
	});
	PostRuntime(TEXT("/runtime/servers/player-joined"), Body, Callback);
}

void UDBA_GameBackendRuntimeService::NotifyPlayerLeft(const FString& PlayerId, const FDBA_GameBackendResponseDelegate& Callback)
{
	const FString Body = BuildRuntimePayload([&PlayerId](TSharedRef<FJsonObject> Json)
	{
		Json->SetStringField(TEXT("playerId"), PlayerId);
	});
	PostRuntime(TEXT("/runtime/servers/player-left"), Body, Callback);
}

void UDBA_GameBackendRuntimeService::NotifyMatchStarted(const FDBA_GameBackendResponseDelegate& Callback)
{
	PostRuntime(TEXT("/runtime/servers/match-started"), BuildRuntimePayload([](TSharedRef<FJsonObject>) {}), Callback);
}

void UDBA_GameBackendRuntimeService::NotifyMatchEnded(const FDBA_GameBackendResponseDelegate& Callback)
{
	PostRuntime(TEXT("/runtime/servers/match-ended"), BuildRuntimePayload([](TSharedRef<FJsonObject>) {}), Callback);
}

FString UDBA_GameBackendRuntimeService::BuildRuntimePayload(const TFunction<void(TSharedRef<FJsonObject>)>& Fill) const
{
	TSharedRef<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField(TEXT("serverId"), ServerId);
	Json->SetStringField(TEXT("sessionId"), SessionId);
	Json->SetStringField(TEXT("runtimeToken"), RuntimeToken);
	if (Fill)
	{
		Fill(Json);
	}

	FString Body;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
	FJsonSerializer::Serialize(Json, Writer);
	return Body;
}

void UDBA_GameBackendRuntimeService::PostRuntime(const FString& Path, const FString& Body, const FDBA_GameBackendResponseDelegate& Callback) const
{
	if (!HttpClient || !IsConfigured())
	{
		Callback.ExecuteIfBound(false, TEXT("Runtime service is not configured."), TEXT("{}"));
		return;
	}

	HttpClient->Post(Path, Body, [Callback](const FDBA_GameBackendHttpResult& Result)
	{
		ExecuteResponse(Callback, Result);
	}, false);
}

void UDBA_GameBackendRuntimeService::ExecuteResponse(const FDBA_GameBackendResponseDelegate& Callback, const FDBA_GameBackendHttpResult& Result)
{
	const bool bSuccess = Result.IsSuccessful();
	const FString ErrorMessage = bSuccess ? FString() : (Result.Message.IsEmpty() ? TEXT("Runtime request failed.") : Result.Message);
	Callback.ExecuteIfBound(bSuccess, ErrorMessage, Result.DataJson);
}
