// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/ServerDirectory/DBAServerDirectorySubsystem.h"

#include "Dom/JsonObject.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "GameCore/Core/DBALogChannels.h"
#include "GameDBA/Frontend/Core/DBAFrontendErrorMapper.h"
#include "GameDBA/Frontend/Online/DBAApiClientSubsystem.h"
#include "Misc/ConfigCacheIni.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
	const TCHAR* ServerDirectoryPreferenceSection = TEXT("DBA.ServerDirectory");
	const TCHAR* LastServerPreferencePrefix = TEXT("LastServerId.");

	FString MakeLastServerPreferenceKey(const FString& AccountId)
	{
		return FString::Printf(TEXT("%s%s"), LastServerPreferencePrefix, *AccountId.TrimStartAndEnd());
	}
}

bool UDBAServerDirectorySubsystem::IsSupportedInCurrentEnvironment() const
{
	return !IsRunningDedicatedServer();
}

bool UDBAServerDirectorySubsystem::RefreshDirectory(const FString& Region, const FString& ClientVersion, const FString& Platform)
{
	if (bRefreshInFlight)
	{
		UE_LOG(LogDBAOnline, Verbose, TEXT("[DBAServerDirectorySubsystem] 区服目录请求正在进行，忽略重复刷新。"));
		return false;
	}

	UDBAApiClientSubsystem* ApiClient = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAApiClientSubsystem>() : nullptr;
	if (!ApiClient)
	{
		const FDBAApiError Error = UDBAFrontendErrorMapper::FromLegacyMessage(TEXT("区服目录服务不可用。"));
		OnDirectoryChanged.Broadcast(false, Error);
		return false;
	}

	bRefreshInFlight = true;
	const uint64 RequestGeneration = ++DirectoryRequestGeneration;
	ApiClient->Get(BuildPath(Region, ClientVersion, Platform), true, this,
		[this, RequestGeneration](const FDBAApiResponse& Response)
		{
			if (RequestGeneration != DirectoryRequestGeneration)
			{
				return;
			}

			bRefreshInFlight = false;
			if (!Response.Result.bSuccess)
			{
				OnDirectoryChanged.Broadcast(false, Response.Result.ApiError);
				return;
			}

			TArray<FDBAServerDirectoryEntry> Servers;
			FString ParseError;
			if (!ParseDirectoryJson(Response.DomainJson, Servers, ParseError))
			{
				const FDBAApiError Error = UDBAFrontendErrorMapper::FromLegacyMessage(ParseError);
				UE_LOG(LogDBAOnline, Warning, TEXT("[DBAServerDirectorySubsystem] 区服目录响应解析失败：%s"), *ParseError);
				OnDirectoryChanged.Broadcast(false, Error);
				return;
			}

			CachedServers = MoveTemp(Servers);
			OnDirectoryChanged.Broadcast(true, FDBAApiError());
		});
	return true;
}

const FDBAServerDirectoryEntry* UDBAServerDirectorySubsystem::FindSelectableServer(const FString& ServerId) const
{
	return CachedServers.FindByPredicate([&ServerId](const FDBAServerDirectoryEntry& Entry)
	{
		return Entry.bCanSelect && Entry.ServerId == ServerId;
	});
}

FString UDBAServerDirectorySubsystem::GetLastSelectedServerId(const FString& AccountId) const
{
	const FString NormalizedAccountId = AccountId.TrimStartAndEnd();
	if (NormalizedAccountId.IsEmpty() || !GConfig)
	{
		return FString();
	}

	FString LastServerId;
	GConfig->GetString(ServerDirectoryPreferenceSection, *MakeLastServerPreferenceKey(NormalizedAccountId), LastServerId, GGameUserSettingsIni);
	return LastServerId;
}

void UDBAServerDirectorySubsystem::RecordLastSelectedServer(const FString& AccountId, const FString& ServerId)
{
	const FString NormalizedAccountId = AccountId.TrimStartAndEnd();
	const FString NormalizedServerId = ServerId.TrimStartAndEnd();
	if (NormalizedAccountId.IsEmpty() || NormalizedServerId.IsEmpty() || !GConfig)
	{
		return;
	}

	GConfig->SetString(ServerDirectoryPreferenceSection, *MakeLastServerPreferenceKey(NormalizedAccountId), *NormalizedServerId, GGameUserSettingsIni);
	GConfig->Flush(false, GGameUserSettingsIni);
}

bool UDBAServerDirectorySubsystem::ParseDirectoryJson(const FString& Json, TArray<FDBAServerDirectoryEntry>& OutServers, FString& OutError)
{
	OutServers.Reset();
	TArray<TSharedPtr<FJsonValue>> Values;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
	if (!FJsonSerializer::Deserialize(Reader, Values))
	{
		OutError = TEXT("区服目录响应不是数组。");
		return false;
	}

	for (const TSharedPtr<FJsonValue>& Value : Values)
	{
		const TSharedPtr<FJsonObject> Object = Value.IsValid() ? Value->AsObject() : nullptr;
		if (!Object.IsValid())
		{
			OutError = TEXT("区服目录包含无效条目。");
			return false;
		}

		FDBAServerDirectoryEntry Entry;
		FString StatusText;
		Object->TryGetStringField(TEXT("serverId"), Entry.ServerId);
		FString Name;
		Object->TryGetStringField(TEXT("name"), Name);
		Entry.Name = FText::FromString(Name);
		Object->TryGetStringField(TEXT("region"), Entry.Region);
		Object->TryGetStringField(TEXT("status"), StatusText);
		Entry.Status = ParseStatus(StatusText);
		Object->TryGetNumberField(TEXT("population"), Entry.Population);
		Object->TryGetBoolField(TEXT("recommended"), Entry.bRecommended);
		FString MaintenanceMessage;
		Object->TryGetStringField(TEXT("maintenanceMessage"), MaintenanceMessage);
		Entry.MaintenanceMessage = FText::FromString(MaintenanceMessage);
		Object->TryGetStringField(TEXT("minClientVersion"), Entry.MinClientVersion);
		Object->TryGetBoolField(TEXT("canSelect"), Entry.bCanSelect);

		if (Entry.ServerId.IsEmpty() || Name.IsEmpty())
		{
			OutError = TEXT("区服目录条目缺少稳定服务器标识或名称。");
			return false;
		}
		OutServers.Add(MoveTemp(Entry));
	}

	return true;
}

EDBAServerDirectoryStatus UDBAServerDirectorySubsystem::ParseStatus(const FString& Value)
{
	if (Value.Equals(TEXT("Online"), ESearchCase::IgnoreCase)) return EDBAServerDirectoryStatus::Online;
	if (Value.Equals(TEXT("Busy"), ESearchCase::IgnoreCase)) return EDBAServerDirectoryStatus::Busy;
	if (Value.Equals(TEXT("Full"), ESearchCase::IgnoreCase)) return EDBAServerDirectoryStatus::Full;
	if (Value.Equals(TEXT("Maintenance"), ESearchCase::IgnoreCase)) return EDBAServerDirectoryStatus::Maintenance;
	return EDBAServerDirectoryStatus::Offline;
}

FString UDBAServerDirectorySubsystem::BuildPath(const FString& Region, const FString& ClientVersion, const FString& Platform)
{
	TArray<FString> Parameters;
	const auto Append = [&Parameters](const TCHAR* Key, const FString& Value)
	{
		if (!Value.TrimStartAndEnd().IsEmpty())
		{
			Parameters.Add(FString::Printf(TEXT("%s=%s"), Key, *FGenericPlatformHttp::UrlEncode(Value.TrimStartAndEnd())));
		}
	};
	Append(TEXT("region"), Region);
	Append(TEXT("clientVersion"), ClientVersion);
	Append(TEXT("platform"), Platform);
	return Parameters.IsEmpty() ? TEXT("/api/v1/servers") : FString::Printf(TEXT("/api/v1/servers?%s"), *FString::Join(Parameters, TEXT("&")));
}
