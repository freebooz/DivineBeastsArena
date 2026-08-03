// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端 / Dedicated Server。
- 文件职责：实现 UE Dedicated Server 到 DBA_GameBackend Runtime API 的 HTTP 上报。
- 阅读重点：Runtime 请求携带 serverId、sessionId、runtimeToken，后端只保存 token hash 并校验绑定关系。
- 修改提示：保持接口无客户端 JWT 依赖；Runtime Token 是服务器专用凭证，不能暴露给普通客户端。
*/

#include "GameBackendRuntimeService.h"

#include "GameBackendClientSubsystem.h"
#include "GameBackendHttpClient.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
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

	BuildId = ReadCommandLineValue(TEXT("buildId="));
	if (BuildId.IsEmpty())
	{
		BuildId = ReadCommandLineValue(TEXT("BuildId="));
	}

	RuntimeToken = ReadCommandLineValue(TEXT("runtimeToken="));
	if (RuntimeToken.IsEmpty())
	{
		RuntimeToken = ReadCommandLineValue(TEXT("RuntimeToken="));
	}

	UE_LOG(LogDBA_GameBackendClient, Log, TEXT("运行时参数读取完成。会话=%s 服务器=%s 构建=%s 已配置令牌=%s"),
		*SessionId,
		*ServerId,
		*BuildId,
		RuntimeToken.IsEmpty() ? TEXT("否") : TEXT("是"));

	return IsConfigured();
}

bool UDBA_GameBackendRuntimeService::IsConfigured() const
{
	return !SessionId.IsEmpty() && !ServerId.IsEmpty() && !BuildId.IsEmpty() && !RuntimeToken.IsEmpty();
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

void UDBA_GameBackendRuntimeService::ValidateJoinTicket(
	const FString& PlayerId,
	const FString& CharacterId,
	const FString& JoinTicket,
	const FString& Team,
	int32 SlotIndex,
	const FDBA_GameBackendRuntimePlayerBuildSummary& BuildSummary,
	FDBA_GameBackendNativeResponseCallback Callback)
{
	const FString Body = BuildRuntimePayload([this, &PlayerId, &CharacterId, &JoinTicket, &Team, SlotIndex, &BuildSummary](TSharedRef<FJsonObject> Json)
	{
		Json->SetStringField(TEXT("playerId"), PlayerId);
		Json->SetStringField(TEXT("characterId"), CharacterId);
		Json->SetStringField(TEXT("buildId"), BuildId);
		Json->SetStringField(TEXT("joinTicket"), JoinTicket);
		Json->SetStringField(TEXT("team"), Team);
		Json->SetNumberField(TEXT("slotIndex"), SlotIndex);
		if (BuildSummary.HasAnyValue())
		{
			Json->SetStringField(TEXT("zodiac"), BuildSummary.Zodiac);
			Json->SetStringField(TEXT("primaryElement"), BuildSummary.PrimaryElement);
			Json->SetStringField(TEXT("fiveCamp"), BuildSummary.FiveCamp);
			Json->SetStringField(TEXT("fixedSkillGroupId"), BuildSummary.FixedSkillGroupId);
		}
	});
	PostRuntimeNative(TEXT("/runtime/servers/validate-join-ticket"), Body, MoveTemp(Callback));
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

void UDBA_GameBackendRuntimeService::NotifyMatchResults(
	const FString& IdempotencyKey,
	const FString& ResultJson,
	const TArray<FDBA_GameBackendRuntimePlayerResult>& Players,
	const FDBA_GameBackendResponseDelegate& Callback)
{
	const FString Body = BuildMatchResultsPayload(ServerId, SessionId, RuntimeToken, IdempotencyKey, ResultJson, Players);
	PostRuntime(TEXT("/runtime/matches/results"), Body, Callback);
}

FString UDBA_GameBackendRuntimeService::BuildMatchResultsPayload(
	const FString& ServerId,
	const FString& SessionId,
	const FString& RuntimeToken,
	const FString& IdempotencyKey,
	const FString& ResultJson,
	const TArray<FDBA_GameBackendRuntimePlayerResult>& Players)
{
	TSharedRef<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField(TEXT("serverId"), ServerId);
	Json->SetStringField(TEXT("sessionId"), SessionId);
	Json->SetStringField(TEXT("runtimeToken"), RuntimeToken);
	Json->SetStringField(TEXT("idempotencyKey"), IdempotencyKey);
	Json->SetStringField(TEXT("resultJson"), ResultJson);

	TArray<TSharedPtr<FJsonValue>> PlayerValues;
	PlayerValues.Reserve(Players.Num());
	for (const FDBA_GameBackendRuntimePlayerResult& Player : Players)
	{
		TSharedRef<FJsonObject> PlayerJson = MakeShared<FJsonObject>();
		PlayerJson->SetStringField(TEXT("playerId"), Player.PlayerId);
		PlayerJson->SetStringField(TEXT("team"), Player.Team);
		PlayerJson->SetStringField(TEXT("result"), Player.Result);
		PlayerJson->SetNumberField(TEXT("kills"), Player.Kills);
		PlayerJson->SetNumberField(TEXT("deaths"), Player.Deaths);
		PlayerJson->SetNumberField(TEXT("assists"), Player.Assists);
		PlayerJson->SetNumberField(TEXT("score"), Player.Score);
		PlayerJson->SetNumberField(TEXT("expDelta"), static_cast<double>(Player.ExpDelta));

		TSharedRef<FJsonObject> RewardsJson = MakeShared<FJsonObject>();
		for (const TPair<FString, int32>& Reward : Player.Rewards)
		{
			RewardsJson->SetNumberField(Reward.Key, Reward.Value);
		}
		PlayerJson->SetObjectField(TEXT("rewards"), RewardsJson);
		PlayerValues.Add(MakeShared<FJsonValueObject>(PlayerJson));
	}
	Json->SetArrayField(TEXT("players"), PlayerValues);

	FString Body;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
	FJsonSerializer::Serialize(Json, Writer);
	return Body;
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
		Callback.ExecuteIfBound(false, TEXT("Runtime 服务未配置。"), TEXT("{}"));
		return;
	}

	HttpClient->Post(Path, Body, [Callback](const FDBA_GameBackendHttpResult& Result)
	{
		ExecuteResponse(Callback, Result);
	}, false);
}

void UDBA_GameBackendRuntimeService::PostRuntimeNative(
	const FString& Path,
	const FString& Body,
	FDBA_GameBackendNativeResponseCallback Callback) const
{
	if (!HttpClient || !IsConfigured())
	{
		if (Callback)
		{
			Callback(false, TEXT("运行时服务未配置。"), TEXT("{}"));
		}
		return;
	}

	HttpClient->Post(Path, Body, [Callback = MoveTemp(Callback)](const FDBA_GameBackendHttpResult& Result) mutable
	{
		const bool bSuccess = Result.IsSuccessful();
		const FString ErrorMessage = bSuccess ? FString() : (Result.Message.IsEmpty() ? TEXT("运行时请求失败。") : Result.Message);
		if (Callback)
		{
			Callback(bSuccess, ErrorMessage, Result.DataJson);
		}
	}, false);
}

void UDBA_GameBackendRuntimeService::ExecuteResponse(const FDBA_GameBackendResponseDelegate& Callback, const FDBA_GameBackendHttpResult& Result)
{
	const bool bSuccess = Result.IsSuccessful();
	const FString ErrorMessage = bSuccess ? FString() : (Result.Message.IsEmpty() ? TEXT("运行时请求失败。") : Result.Message);
	Callback.ExecuteIfBound(bSuccess, ErrorMessage, Result.DataJson);
}
