// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient / GameBackendClient Unreal 插件。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameBackendPlayerService.h"

#include "GameBackendClientSubsystem.h"
#include "GameBackendHttpClient.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace
{
	void ExecuteResponse(const FDBA_GameBackendResponseDelegate& Callback, const FDBA_GameBackendHttpResult& Result)
	{
		const bool bSuccess = Result.IsSuccessful();
		const FString ErrorMessage = bSuccess ? FString() : (Result.Message.IsEmpty() ? TEXT("请求失败。") : Result.Message);
		Callback.ExecuteIfBound(bSuccess, ErrorMessage, Result.DataJson);
	}

	TSharedPtr<FJsonObject> ResolvePayloadObject(const TSharedPtr<FJsonObject>& Root)
	{
		if (!Root.IsValid())
		{
			return nullptr;
		}

		const TSharedPtr<FJsonObject>* DataObj = nullptr;
		if (Root->TryGetObjectField(TEXT("data"), DataObj) && DataObj && DataObj->IsValid())
		{
			return *DataObj;
		}

		return Root;
	}

	FString ReadStringField(const TSharedPtr<FJsonObject>& Object, const TCHAR* FieldName)
	{
		FString Value;
		if (Object.IsValid())
		{
			Object->TryGetStringField(FieldName, Value);
		}
		return Value;
	}

	int32 ReadIntField(const TSharedPtr<FJsonObject>& Object, const TCHAR* FieldName, int32 DefaultValue = 0)
	{
		double Value = static_cast<double>(DefaultValue);
		if (Object.IsValid())
		{
			Object->TryGetNumberField(FieldName, Value);
		}
		return static_cast<int32>(Value);
	}

	int64 ReadInt64Field(const TSharedPtr<FJsonObject>& Object, const TCHAR* FieldName, int64 DefaultValue = 0)
	{
		double Value = static_cast<double>(DefaultValue);
		if (Object.IsValid())
		{
			Object->TryGetNumberField(FieldName, Value);
		}
		return static_cast<int64>(Value);
	}

	void ReadNumericRewards(const TSharedPtr<FJsonObject>& MatchObj, TMap<FString, int64>& OutRewards)
	{
		OutRewards.Reset();
		const TSharedPtr<FJsonObject>* RewardsObj = nullptr;
		if (!MatchObj.IsValid() ||
			!MatchObj->TryGetObjectField(TEXT("rewards"), RewardsObj) ||
			!RewardsObj ||
			!RewardsObj->IsValid())
		{
			return;
		}

		for (const TPair<FString, TSharedPtr<FJsonValue>>& Reward : (*RewardsObj)->Values)
		{
			if (!Reward.Value.IsValid() || Reward.Value->Type != EJson::Number)
			{
				continue;
			}

			OutRewards.Add(Reward.Key, static_cast<int64>(Reward.Value->AsNumber()));
		}
	}
}

void UDBA_GameBackendPlayerService::Initialize(UDBA_GameBackendClientSubsystem* InSubsystem, FDBA_GameBackendHttpClient* InHttpClient)
{
	Subsystem = InSubsystem;
	HttpClient = InHttpClient;
}

void UDBA_GameBackendPlayerService::GetMyProfile(const FDBA_GameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("HTTP 客户端不可用。"), TEXT("{}"));
		return;
	}
	HttpClient->Get(TEXT("/api/players/me/profile"), [Callback](const FDBA_GameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UDBA_GameBackendPlayerService::UpdateMyProfile(const FString& ProfileJson, const FDBA_GameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("HTTP 客户端不可用。"), TEXT("{}"));
		return;
	}
	HttpClient->Patch(TEXT("/api/players/me/profile"), ProfileJson.IsEmpty() ? TEXT("{}") : ProfileJson, [Callback](const FDBA_GameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UDBA_GameBackendPlayerService::GetMySettings(const FDBA_GameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("HTTP 客户端不可用。"), TEXT("{}"));
		return;
	}
	HttpClient->Get(TEXT("/api/players/me/settings"), [Callback](const FDBA_GameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UDBA_GameBackendPlayerService::UpdateMySettings(const FString& SettingsJson, const FDBA_GameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("HTTP 客户端不可用。"), TEXT("{}"));
		return;
	}
	HttpClient->Put(TEXT("/api/players/me/settings"), SettingsJson.IsEmpty() ? TEXT("{}") : SettingsJson, [Callback](const FDBA_GameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UDBA_GameBackendPlayerService::GetMyStats(const FDBA_GameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("HTTP 客户端不可用。"), TEXT("{}"));
		return;
	}
	HttpClient->Get(TEXT("/api/players/me/stats"), [Callback](const FDBA_GameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UDBA_GameBackendPlayerService::GetMyUnlocks(const FDBA_GameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("HTTP 客户端不可用。"), TEXT("{}"));
		return;
	}
	HttpClient->Get(TEXT("/api/players/me/unlocks"), [Callback](const FDBA_GameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UDBA_GameBackendPlayerService::GetMyInventory(const FDBA_GameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("HTTP 客户端不可用。"), TEXT("{}"));
		return;
	}
	HttpClient->Get(TEXT("/api/players/me/inventory"), [Callback](const FDBA_GameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UDBA_GameBackendPlayerService::GetMyMatches(const FDBA_GameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("HTTP 客户端不可用。"), TEXT("{}"));
		return;
	}
	HttpClient->Get(TEXT("/api/players/me/matches"), [Callback](const FDBA_GameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

bool UDBA_GameBackendPlayerService::TryParseMatchHistoryData(const FString& DataJson, FDBA_GameBackendMatchHistoryPage& OutPage, FString& OutError)
{
	OutPage = FDBA_GameBackendMatchHistoryPage();
	OutError.Reset();

	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(DataJson);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		OutError = TEXT("战绩历史 JSON 无效。");
		return false;
	}

	const TSharedPtr<FJsonObject> Payload = ResolvePayloadObject(Root);
	if (!Payload.IsValid())
	{
		OutError = TEXT("战绩历史载荷缺失。");
		return false;
	}

	OutPage.TotalCount = ReadIntField(Payload, TEXT("totalCount"));
	OutPage.Page = ReadIntField(Payload, TEXT("page"), 1);
	OutPage.PageSize = ReadIntField(Payload, TEXT("pageSize"), 50);

	const TArray<TSharedPtr<FJsonValue>>* Matches = nullptr;
	if (!Payload->TryGetArrayField(TEXT("matches"), Matches) || !Matches)
	{
		OutError = TEXT("战绩历史载荷缺少 matches 列表。");
		return false;
	}

	for (const TSharedPtr<FJsonValue>& MatchValue : *Matches)
	{
		const TSharedPtr<FJsonObject> MatchObj = MatchValue.IsValid() ? MatchValue->AsObject() : nullptr;
		if (!MatchObj.IsValid())
		{
			continue;
		}

		FDBA_GameBackendMatchHistoryEntry Entry;
		Entry.SessionId = ReadStringField(MatchObj, TEXT("sessionId"));
		Entry.Mode = ReadStringField(MatchObj, TEXT("mode"));
		Entry.MapId = ReadStringField(MatchObj, TEXT("mapId"));
		Entry.Team = ReadStringField(MatchObj, TEXT("team"));
		Entry.Result = ReadStringField(MatchObj, TEXT("result"));
		Entry.Kills = ReadIntField(MatchObj, TEXT("kills"));
		Entry.Deaths = ReadIntField(MatchObj, TEXT("deaths"));
		Entry.Assists = ReadIntField(MatchObj, TEXT("assists"));
		Entry.Score = ReadIntField(MatchObj, TEXT("score"));
		Entry.ResultJson = ReadStringField(MatchObj, TEXT("resultJson"));
		Entry.WinnerTeam = ReadStringField(MatchObj, TEXT("winnerTeam"));
		Entry.DurationSeconds = ReadIntField(MatchObj, TEXT("durationSeconds"));
		Entry.ExpDelta = ReadInt64Field(MatchObj, TEXT("expDelta"));
		Entry.PlayedAtUtc = ReadStringField(MatchObj, TEXT("playedAt"));
		ReadNumericRewards(MatchObj, Entry.Rewards);
		OutPage.Matches.Add(MoveTemp(Entry));
	}

	return true;
}
