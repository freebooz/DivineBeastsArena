// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient / GameBackendClient Unreal 插件。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameBackendSessionService.h"

#include "Dom/JsonObject.h"
#include "GameBackendClientSubsystem.h"
#include "GameBackendHttpClient.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace
{
	void AppendTravelOption(FString& Url, const FString& Key, const FString& Value)
	{
		if (Value.IsEmpty())
		{
			return;
		}

		Url += Url.Contains(TEXT("?")) ? TEXT("&") : TEXT("?");
		Url += FString::Printf(TEXT("%s=%s"), *Key, *FGenericPlatformHttp::UrlEncode(Value));
	}

	void ExecuteResponse(const FDBA_GameBackendResponseDelegate& Callback, const FDBA_GameBackendHttpResult& Result)
	{
		const bool bSuccess = Result.IsSuccessful();
		const FString ErrorMessage = bSuccess ? FString() : (Result.Message.IsEmpty() ? TEXT("请求失败。") : Result.Message);
		Callback.ExecuteIfBound(bSuccess, ErrorMessage, Result.DataJson);
	}
}

void UDBA_GameBackendSessionService::Initialize(UDBA_GameBackendClientSubsystem* InSubsystem, FDBA_GameBackendHttpClient* InHttpClient)
{
	Subsystem = InSubsystem;
	HttpClient = InHttpClient;
}

void UDBA_GameBackendSessionService::GetSession(const FString& SessionId, const FDBA_GameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("会话服务不可用。"), TEXT("{}"));
		return;
	}

	HttpClient->Get(FString::Printf(TEXT("/api/sessions/%s"), *SessionId), [Callback](const FDBA_GameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UDBA_GameBackendSessionService::GetConnection(const FString& SessionId, const FDBA_GameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("会话服务不可用。"), TEXT("{}"));
		return;
	}

	HttpClient->Get(FString::Printf(TEXT("/api/sessions/%s/connection"), *SessionId), [Callback](const FDBA_GameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UDBA_GameBackendSessionService::AllocateVillage(const FString& CharacterId, const FDBA_GameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("新手村分配服务不可用。"), TEXT("{}"));
		return;
	}
	if (CharacterId.TrimStartAndEnd().IsEmpty())
	{
		Callback.ExecuteIfBound(false, TEXT("角色标识为空，无法分配新手村。"), TEXT("{}"));
		return;
	}

	TSharedRef<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField(TEXT("characterId"), CharacterId);
	FString BodyJson;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&BodyJson);
	FJsonSerializer::Serialize(Json, Writer);
	HttpClient->Post(TEXT("/api/sessions/village-allocation"), BodyJson,
		[Callback](const FDBA_GameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UDBA_GameBackendSessionService::RequestReconnectToken(const FString& SessionId, const FDBA_GameBackendResponseDelegate& Callback)
{
	// 兼容旧调用：未提供 ReconnectToken 时无法完成重连，直接失败。
	UE_LOG(LogDBA_GameBackendClient, Warning, TEXT("[会话] RequestReconnectToken 已废弃，请改用 Reconnect(SessionId, ReconnectToken, Callback)。"));
	Callback.ExecuteIfBound(false, TEXT("请使用 Reconnect 方法并传入持久化的重连令牌。"), TEXT("{}"));
}

void UDBA_GameBackendSessionService::Reconnect(const FString& SessionId, const FString& ReconnectToken, const FDBA_GameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("会话服务不可用。"), TEXT("{}"));
		return;
	}

	if (ReconnectToken.IsEmpty())
	{
		Callback.ExecuteIfBound(false, TEXT("重连令牌为空，无法执行断线重连。"), TEXT("{}"));
		return;
	}

	// 构造请求体：{"reconnectToken":"<token>"}
	const FString BodyJson = FString::Printf(TEXT("{\"reconnectToken\":\"%s\"}"), *ReconnectToken);
	HttpClient->Post(
		FString::Printf(TEXT("/api/sessions/%s/reconnect"), *SessionId),
		BodyJson,
		[Callback](const FDBA_GameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

FString UDBA_GameBackendSessionService::BuildTravelUrl(const FString& Ip, int32 Port, const FString& SessionId, const FString& JoinTicket)
{
	const FString TrimmedIp = Ip.TrimStartAndEnd();
	if (TrimmedIp.IsEmpty() || Port <= 0)
	{
		return FString();
	}

	FString Url = FString::Printf(TEXT("%s:%d"), *TrimmedIp, Port);
	AppendTravelOption(Url, TEXT("SessionId"), SessionId);
	AppendTravelOption(Url, TEXT("JoinTicket"), JoinTicket);
	return Url;
}

FString UDBA_GameBackendSessionService::BuildTravelUrl(
	const FString& Ip,
	int32 Port,
	const FString& SessionId,
	const FString& JoinTicket,
	int32 TeamId,
	const FString& Zodiac,
	const FString& PrimaryElement,
	const FString& FiveCamp,
	const FString& FixedSkillGroupId)
{
	FString Url = BuildTravelUrl(Ip, Port, SessionId, JoinTicket);
	if (TeamId > 0)
	{
		AppendTravelOption(Url, TEXT("DBATeamId"), FString::FromInt(TeamId));
	}
	AppendTravelOption(Url, TEXT("DBAZodiac"), Zodiac);
	AppendTravelOption(Url, TEXT("DBAElement"), PrimaryElement);
	AppendTravelOption(Url, TEXT("DBAFiveCamp"), FiveCamp);
	AppendTravelOption(Url, TEXT("DBAFixedSkillGroupId"), FixedSkillGroupId);
	return Url;
}

FString UDBA_GameBackendSessionService::BuildTravelUrl(
	const FString& Ip,
	int32 Port,
	const FString& SessionId,
	const FString& JoinTicket,
	const FString& PlayerId,
	const FString& CharacterId,
	int32 TeamId,
	const FString& Zodiac,
	const FString& PrimaryElement,
	const FString& FiveCamp,
	const FString& FixedSkillGroupId)
{
	FString Url = BuildTravelUrl(
		Ip,
		Port,
		SessionId,
		JoinTicket,
		TeamId,
		Zodiac,
		PrimaryElement,
		FiveCamp,
		FixedSkillGroupId);
	AppendTravelOption(Url, TEXT("PlayerId"), PlayerId);
	AppendTravelOption(Url, TEXT("CharacterId"), CharacterId);
	return Url;
}

bool UDBA_GameBackendSessionService::TryBuildTravelUrlFromConnectionData(
	const FString& ConnectionDataJson,
	const FString& OverrideSessionId,
	FString& OutTravelUrl)
{
	FDBA_GameBackendSessionConnection Connection;
	if (!ParseConnectionData(ConnectionDataJson, Connection))
	{
		OutTravelUrl.Reset();
		return false;
	}

	OutTravelUrl = BuildTravelUrl(
		Connection.Ip,
		Connection.Port,
		OverrideSessionId.IsEmpty() ? Connection.SessionId : OverrideSessionId,
		Connection.JoinTicket,
		Connection.PlayerId,
		Connection.CharacterId,
		Connection.TeamId,
		Connection.Zodiac,
		Connection.PrimaryElement,
		Connection.FiveCamp,
		Connection.FixedSkillGroupId);
	return !OutTravelUrl.IsEmpty();
}

bool UDBA_GameBackendSessionService::ParseConnectionData(const FString& DataJson, FDBA_GameBackendSessionConnection& OutConnection)
{
	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(DataJson);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		return false;
	}

	TSharedPtr<FJsonObject> Payload = Root;
	const TSharedPtr<FJsonObject>* DataObj = nullptr;
	if (Root->TryGetObjectField(TEXT("data"), DataObj) && DataObj && DataObj->IsValid())
	{
		Payload = *DataObj;
	}

	Payload->TryGetStringField(TEXT("ip"), OutConnection.Ip);
	Payload->TryGetNumberField(TEXT("port"), OutConnection.Port);
	Payload->TryGetStringField(TEXT("sessionId"), OutConnection.SessionId);
	Payload->TryGetStringField(TEXT("joinTicket"), OutConnection.JoinTicket);
	Payload->TryGetStringField(TEXT("playerSessionToken"), OutConnection.PlayerSessionToken);
	Payload->TryGetStringField(TEXT("playerId"), OutConnection.PlayerId);
	Payload->TryGetStringField(TEXT("characterId"), OutConnection.CharacterId);
	Payload->TryGetStringField(TEXT("serverInstanceId"), OutConnection.ServerInstanceId);
	Payload->TryGetStringField(TEXT("buildId"), OutConnection.BuildId);
	Payload->TryGetNumberField(TEXT("teamId"), OutConnection.TeamId);
	Payload->TryGetStringField(TEXT("reconnectToken"), OutConnection.ReconnectToken);
	Payload->TryGetStringField(TEXT("reconnectTokenExpiresAt"), OutConnection.ReconnectTokenExpiresAt);

	const TSharedPtr<FJsonObject>* BuildSummaryObj = nullptr;
	if (Payload->TryGetObjectField(TEXT("characterBuildSummary"), BuildSummaryObj) && BuildSummaryObj && BuildSummaryObj->IsValid())
	{
		(*BuildSummaryObj)->TryGetStringField(TEXT("zodiac"), OutConnection.Zodiac);
		(*BuildSummaryObj)->TryGetStringField(TEXT("primaryElement"), OutConnection.PrimaryElement);
		(*BuildSummaryObj)->TryGetStringField(TEXT("fiveCamp"), OutConnection.FiveCamp);
		(*BuildSummaryObj)->TryGetStringField(TEXT("fixedSkillGroupId"), OutConnection.FixedSkillGroupId);
	}

	if (OutConnection.Ip.IsEmpty())
	{
		Payload->TryGetStringField(TEXT("serverIp"), OutConnection.Ip);
	}
	if (OutConnection.Port <= 0)
	{
		Payload->TryGetNumberField(TEXT("serverPort"), OutConnection.Port);
	}
	if (OutConnection.JoinTicket.IsEmpty())
	{
		OutConnection.JoinTicket = OutConnection.PlayerSessionToken;
	}
	if (OutConnection.JoinTicket.IsEmpty())
	{
		Payload->TryGetStringField(TEXT("sessionToken"), OutConnection.JoinTicket);
	}

	if (OutConnection.Ip.IsEmpty() && Payload->HasTypedField<EJson::Object>(TEXT("connection")))
	{
		const TSharedPtr<FJsonObject> ConnectionObj = Payload->GetObjectField(TEXT("connection"));
		if (ConnectionObj.IsValid())
		{
			ConnectionObj->TryGetStringField(TEXT("ip"), OutConnection.Ip);
			ConnectionObj->TryGetNumberField(TEXT("port"), OutConnection.Port);
			ConnectionObj->TryGetStringField(TEXT("sessionId"), OutConnection.SessionId);
			ConnectionObj->TryGetStringField(TEXT("joinTicket"), OutConnection.JoinTicket);
			ConnectionObj->TryGetStringField(TEXT("playerSessionToken"), OutConnection.PlayerSessionToken);
			ConnectionObj->TryGetStringField(TEXT("playerId"), OutConnection.PlayerId);
			ConnectionObj->TryGetStringField(TEXT("characterId"), OutConnection.CharacterId);
			ConnectionObj->TryGetStringField(TEXT("serverInstanceId"), OutConnection.ServerInstanceId);
			ConnectionObj->TryGetStringField(TEXT("buildId"), OutConnection.BuildId);
			ConnectionObj->TryGetNumberField(TEXT("teamId"), OutConnection.TeamId);
			ConnectionObj->TryGetStringField(TEXT("reconnectToken"), OutConnection.ReconnectToken);
			ConnectionObj->TryGetStringField(TEXT("reconnectTokenExpiresAt"), OutConnection.ReconnectTokenExpiresAt);
			if (OutConnection.Ip.IsEmpty())
			{
				ConnectionObj->TryGetStringField(TEXT("serverIp"), OutConnection.Ip);
			}
			if (OutConnection.Port <= 0)
			{
				ConnectionObj->TryGetNumberField(TEXT("serverPort"), OutConnection.Port);
			}
			if (OutConnection.JoinTicket.IsEmpty())
			{
				OutConnection.JoinTicket = OutConnection.PlayerSessionToken;
			}
			if (OutConnection.JoinTicket.IsEmpty())
			{
				ConnectionObj->TryGetStringField(TEXT("sessionToken"), OutConnection.JoinTicket);
			}

			const TSharedPtr<FJsonObject>* NestedBuildSummaryObj = nullptr;
			if (ConnectionObj->TryGetObjectField(TEXT("characterBuildSummary"), NestedBuildSummaryObj) && NestedBuildSummaryObj && NestedBuildSummaryObj->IsValid())
			{
				(*NestedBuildSummaryObj)->TryGetStringField(TEXT("zodiac"), OutConnection.Zodiac);
				(*NestedBuildSummaryObj)->TryGetStringField(TEXT("primaryElement"), OutConnection.PrimaryElement);
				(*NestedBuildSummaryObj)->TryGetStringField(TEXT("fiveCamp"), OutConnection.FiveCamp);
				(*NestedBuildSummaryObj)->TryGetStringField(TEXT("fixedSkillGroupId"), OutConnection.FixedSkillGroupId);
			}
		}
	}

	return !OutConnection.Ip.IsEmpty()
		&& OutConnection.Port > 0
		&& !OutConnection.SessionId.IsEmpty()
		&& !OutConnection.JoinTicket.IsEmpty()
		&& !OutConnection.PlayerId.IsEmpty()
		&& !OutConnection.CharacterId.IsEmpty()
		&& !OutConnection.ServerInstanceId.IsEmpty()
		&& !OutConnection.BuildId.IsEmpty();
}
