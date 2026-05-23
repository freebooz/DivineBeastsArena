// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端 / Dedicated Server。
- 文件职责：实现 UE Dedicated Server 到 DBA_GameBackend Runtime API 的 HTTP 上报。
- 阅读重点：所有 Runtime 请求都带 serverId、sessionId、runtimeToken，后端只保存 token hash 并校验绑定关系。
- 修改提示：保持接口无客户端 JWT 依赖，Runtime Token 是服务器专用凭证，不能暴露给普通客户端。
*/

#include "GameBackendRuntimeService.h"

#include "DBA_GameBackendClientSubsystem.h"
#include "DBA_GameBackendHttpClient.h"
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

	UE_LOG(LogDBA_GameBackendClient, Log, TEXT("Runtime 参数读取完成。SessionId=%s ServerId=%s 已配置Token=%s"),
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
