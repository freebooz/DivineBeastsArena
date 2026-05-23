// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameBackendConfigService.h"

#include "GameBackendClientSubsystem.h"
#include "GameBackendHttpClient.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace
{
	void ExecuteResponse(const FDBA_GameBackendResponseDelegate& Callback, const FDBA_GameBackendHttpResult& Result)
	{
		const bool bSuccess = Result.bHttpRequestOk && Result.HttpStatus >= 200 && Result.HttpStatus < 300;
		const FString ErrorMessage = bSuccess ? FString() : (Result.Message.IsEmpty() ? TEXT("Request failed.") : Result.Message);
		Callback.ExecuteIfBound(bSuccess, ErrorMessage, Result.DataJson);
	}
}

void UDBA_GameBackendConfigService::Initialize(UDBA_GameBackendClientSubsystem* InSubsystem, FDBA_GameBackendHttpClient* InHttpClient)
{
	Subsystem = InSubsystem;
	HttpClient = InHttpClient;
}

void UDBA_GameBackendConfigService::VersionCheck(const FDBA_GameBackendResponseDelegate& Callback)
{
	if (!HttpClient || !Subsystem.IsValid())
	{
		Callback.ExecuteIfBound(false, TEXT("Config service unavailable."), TEXT("{}"));
		return;
	}

	const TSharedRef<FJsonObject> Request = MakeShared<FJsonObject>();
	Request->SetStringField(TEXT("clientVersion"), Subsystem->GetClientVersion());
	Request->SetStringField(TEXT("platform"), Subsystem->GetPlatformName());

	FString Body;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
	FJsonSerializer::Serialize(Request, Writer);

	HttpClient->Post(TEXT("/api/client/version-check"), Body, [Callback](const FDBA_GameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); }, false);
}

void UDBA_GameBackendConfigService::GetMaintenanceStatus(const FDBA_GameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("Config service unavailable."), TEXT("{}"));
		return;
	}
	HttpClient->Get(TEXT("/api/maintenance/status"), [Callback](const FDBA_GameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); }, false);
}

void UDBA_GameBackendConfigService::GetAnnouncementsPopup(const FDBA_GameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("Config service unavailable."), TEXT("{}"));
		return;
	}
	HttpClient->Get(TEXT("/api/announcements/popup"), [Callback](const FDBA_GameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); }, false);
}

void UDBA_GameBackendConfigService::GetConfigManifest(const FDBA_GameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("Config service unavailable."), TEXT("{}"));
		return;
	}
	HttpClient->Get(TEXT("/api/config/manifest"), [Callback](const FDBA_GameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UDBA_GameBackendConfigService::GetConfigBundle(const FDBA_GameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("Config service unavailable."), TEXT("{}"));
		return;
	}
	HttpClient->Get(TEXT("/api/config/bundle"), [Callback](const FDBA_GameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UDBA_GameBackendConfigService::GetConfigByKey(const FString& ConfigKey, const FDBA_GameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("Config service unavailable."), TEXT("{}"));
		return;
	}
	const FString SafeKey = ConfigKey.TrimStartAndEnd();
	HttpClient->Get(FString::Printf(TEXT("/api/config/%s"), *SafeKey), [Callback](const FDBA_GameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

