// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient / GameBackendClient Unreal 插件。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameBackendPaymentService.h"

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
}

void UDBA_GameBackendPaymentService::Initialize(UDBA_GameBackendClientSubsystem* InSubsystem, FDBA_GameBackendHttpClient* InHttpClient)
{
	Subsystem = InSubsystem;
	HttpClient = InHttpClient;
}

void UDBA_GameBackendPaymentService::CreateOrder(const FString& ProductId, const FString& Platform, const FDBA_GameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("支付服务不可用。"), TEXT("{}"));
		return;
	}

	const TSharedRef<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField(TEXT("productId"), ProductId);
	Json->SetStringField(TEXT("platform"), Platform);

	FString Body;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
	FJsonSerializer::Serialize(Json, Writer);

	HttpClient->Post(TEXT("/api/payments/create-order"), Body, [Callback](const FDBA_GameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UDBA_GameBackendPaymentService::GetOrderDetail(const FString& OrderId, const FDBA_GameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("支付服务不可用。"), TEXT("{}"));
		return;
	}
	HttpClient->Get(FString::Printf(TEXT("/api/payments/orders/%s"), *OrderId), [Callback](const FDBA_GameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}

void UDBA_GameBackendPaymentService::GetMyOrders(const FDBA_GameBackendResponseDelegate& Callback)
{
	if (!HttpClient)
	{
		Callback.ExecuteIfBound(false, TEXT("支付服务不可用。"), TEXT("{}"));
		return;
	}
	HttpClient->Get(TEXT("/api/payments/orders"), [Callback](const FDBA_GameBackendHttpResult& Result) { ExecuteResponse(Callback, Result); });
}
