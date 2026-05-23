// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameBackendSupportService.h"

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

	FString BuildPayload(const TFunction<void(TSharedRef<FJsonObject>)>& Fill)
	{
		const TSharedRef<FJsonObject> Json = MakeShared<FJsonObject>();
		Fill(Json);
		FString Body;
		const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
		FJsonSerializer::Serialize(Json, Writer);
		return Body;
	}
}

void UDBA_GameBackendSupportService::Initialize(UDBA_GameBackendClientSubsystem* InSubsystem, FDBA_GameBackendHttpClient* InHttpClient)
{
	Subsystem = InSubsystem;
	HttpClient = InHttpClient;
}

void UDBA_GameBackendSupportService::SubmitTicket(const FDBA_GameBackendSupportTicketRequest& Request, const FDBA_GameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("客服服务不可用。"), TEXT("{}"));
		return;
	}

	const FString Body = BuildPayload([&Request](TSharedRef<FJsonObject> Json)
	{
		Json->SetStringField(TEXT("subject"), Request.Subject);
		Json->SetStringField(TEXT("category"), Request.Category);
		Json->SetStringField(TEXT("content"), Request.Content);
	});

	HttpClient->Post(TEXT("/api/support/tickets"), Body, [Callback](const FDBA_GameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UDBA_GameBackendSupportService::GetMyTickets(const FDBA_GameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("客服服务不可用。"), TEXT("{}"));
		return;
	}
	HttpClient->Get(TEXT("/api/support/tickets/me"), [Callback](const FDBA_GameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UDBA_GameBackendSupportService::SubmitReport(const FDBA_GameBackendReportRequest& Request, const FDBA_GameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("举报服务不可用。"), TEXT("{}"));
		return;
	}

	const FString Body = BuildPayload([&Request](TSharedRef<FJsonObject> Json)
	{
		Json->SetStringField(TEXT("targetPlayerId"), Request.TargetPlayerId);
		Json->SetStringField(TEXT("reasonCode"), Request.ReasonCode);
		Json->SetStringField(TEXT("description"), Request.Description);
	});

	HttpClient->Post(TEXT("/api/reports"), Body, [Callback](const FDBA_GameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UDBA_GameBackendSupportService::GetMyReports(const FDBA_GameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("举报服务不可用。"), TEXT("{}"));
		return;
	}
	HttpClient->Get(TEXT("/api/reports/me"), [Callback](const FDBA_GameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UDBA_GameBackendSupportService::SubmitAppeal(const FString& AppealJson, const FDBA_GameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("申诉服务不可用。"), TEXT("{}"));
		return;
	}
	HttpClient->Post(TEXT("/api/appeals"), AppealJson.IsEmpty() ? TEXT("{}") : AppealJson, [Callback](const FDBA_GameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}
