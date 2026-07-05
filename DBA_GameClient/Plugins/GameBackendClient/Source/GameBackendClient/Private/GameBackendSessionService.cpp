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
#include "Engine/World.h"
#include "GameBackendClientSubsystem.h"
#include "GameBackendHttpClient.h"
#include "GameFramework/PlayerController.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

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

void UDBA_GameBackendSessionService::RequestReconnectToken(const FString& SessionId, const FDBA_GameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("会话服务不可用。"), TEXT("{}"));
		return;
	}

	HttpClient->Post(FString::Printf(TEXT("/api/sessions/%s/reconnect-token"), *SessionId), TEXT("{}"), [Callback](const FDBA_GameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UDBA_GameBackendSessionService::ConnectToDedicatedServer(const FString& SessionId, const FDBA_GameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("会话服务不可用。"), TEXT("{}"));
		return;
	}

	HttpClient->Get(FString::Printf(TEXT("/api/sessions/%s/connection"), *SessionId), [this, SessionId, Callback](const FDBA_GameBackendHttpResult& Result)
	{
		const bool bSuccess = Result.IsSuccessful();
		if (!bSuccess)
		{
			Callback.ExecuteIfBound(false, Result.Message.IsEmpty() ? TEXT("获取会话连接失败。") : Result.Message, TEXT("{}"));
			return;
		}

		ConnectToDedicatedServer(SessionId, Result.DataJson, Callback);
	});
}

void UDBA_GameBackendSessionService::ConnectToDedicatedServer(const FString& SessionId, const FString& ConnectionDataJson, const FDBA_GameBackendResponseDelegate& Callback)
{
	FString TravelUrl;
	if (!TryBuildTravelUrlFromConnectionData(ConnectionDataJson, SessionId, TravelUrl))
	{
		Callback.ExecuteIfBound(false, TEXT("连接数据无效。"), TEXT("{}"));
		return;
	}

	if (Subsystem.IsValid())
	{
		FDBA_GameBackendSessionConnection Connection;
		ParseConnectionData(ConnectionDataJson, Connection);
		AppendTravelOption(TravelUrl, TEXT("PlayerId"), Connection.PlayerId.IsEmpty() ? Subsystem->GetPlayerId() : Connection.PlayerId);
	}
	if (TravelUrl.IsEmpty())
	{
		Callback.ExecuteIfBound(false, TEXT("构建跳转 URL 失败。"), TEXT("{}"));
		return;
	}

	if (!Subsystem.IsValid() || !Subsystem->GetGameInstance())
	{
		Callback.ExecuteIfBound(false, TEXT("GameInstance 不可用。"), TEXT("{}"));
		return;
	}

	UWorld* World = Subsystem->GetGameInstance()->GetWorld();
	APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	if (!PC)
	{
		Callback.ExecuteIfBound(false, TEXT("PlayerController 不可用。"), TEXT("{}"));
		return;
	}

	PC->ClientTravel(TravelUrl, TRAVEL_Absolute);
	Callback.ExecuteIfBound(true, FString(), TravelUrl);
}

FString UDBA_GameBackendSessionService::BuildTravelUrl(const FString& Ip, int32 Port, const FString& SessionId, const FString& PlayerSessionToken)
{
	const FString TrimmedIp = Ip.TrimStartAndEnd();
	if (TrimmedIp.IsEmpty() || Port <= 0)
	{
		return FString();
	}

	FString Url = FString::Printf(TEXT("%s:%d"), *TrimmedIp, Port);
	AppendTravelOption(Url, TEXT("SessionId"), SessionId);
	AppendTravelOption(Url, TEXT("PlayerSessionToken"), PlayerSessionToken);
	return Url;
}

FString UDBA_GameBackendSessionService::BuildTravelUrl(
	const FString& Ip,
	int32 Port,
	const FString& SessionId,
	const FString& PlayerSessionToken,
	int32 TeamId,
	const FString& Zodiac,
	const FString& PrimaryElement,
	const FString& FiveCamp,
	const FString& FixedSkillGroupId)
{
	FString Url = BuildTravelUrl(Ip, Port, SessionId, PlayerSessionToken);
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
		Connection.PlayerSessionToken,
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
	Payload->TryGetStringField(TEXT("playerSessionToken"), OutConnection.PlayerSessionToken);
	Payload->TryGetStringField(TEXT("playerId"), OutConnection.PlayerId);
	Payload->TryGetNumberField(TEXT("teamId"), OutConnection.TeamId);

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
	if (OutConnection.PlayerSessionToken.IsEmpty())
	{
		Payload->TryGetStringField(TEXT("sessionToken"), OutConnection.PlayerSessionToken);
	}

	if (OutConnection.Ip.IsEmpty() && Payload->HasTypedField<EJson::Object>(TEXT("connection")))
	{
		const TSharedPtr<FJsonObject> ConnectionObj = Payload->GetObjectField(TEXT("connection"));
		if (ConnectionObj.IsValid())
		{
			ConnectionObj->TryGetStringField(TEXT("ip"), OutConnection.Ip);
			ConnectionObj->TryGetNumberField(TEXT("port"), OutConnection.Port);
			ConnectionObj->TryGetStringField(TEXT("sessionId"), OutConnection.SessionId);
			ConnectionObj->TryGetStringField(TEXT("playerSessionToken"), OutConnection.PlayerSessionToken);
			ConnectionObj->TryGetStringField(TEXT("playerId"), OutConnection.PlayerId);
			ConnectionObj->TryGetNumberField(TEXT("teamId"), OutConnection.TeamId);
			if (OutConnection.Ip.IsEmpty())
			{
				ConnectionObj->TryGetStringField(TEXT("serverIp"), OutConnection.Ip);
			}
			if (OutConnection.Port <= 0)
			{
				ConnectionObj->TryGetNumberField(TEXT("serverPort"), OutConnection.Port);
			}
			if (OutConnection.PlayerSessionToken.IsEmpty())
			{
				ConnectionObj->TryGetStringField(TEXT("sessionToken"), OutConnection.PlayerSessionToken);
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

	return !OutConnection.Ip.IsEmpty() && OutConnection.Port > 0;
}
