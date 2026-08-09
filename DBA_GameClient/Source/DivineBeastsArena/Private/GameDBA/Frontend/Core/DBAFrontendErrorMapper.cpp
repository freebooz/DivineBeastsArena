// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/Core/DBAFrontendErrorMapper.h"

namespace
{
	FDBAApiError CreateApiError(const FName ErrorCode, const EDBANetworkErrorCategory Category, const FText& UserMessage, const int32 HttpStatusCode, const bool bCanRetry)
	{
		FDBAApiError Error;
		Error.ErrorCode = ErrorCode;
		Error.Category = Category;
		Error.UserMessage = UserMessage;
		Error.HttpStatusCode = HttpStatusCode;
		Error.bCanRetry = bCanRetry;
		return Error;
	}
}

FDBAApiError UDBAFrontendErrorMapper::FromHttpStatus(const int32 HttpStatusCode, const FName BackendErrorCode)
{
	if (BackendErrorCode == TEXT("character.name_taken"))
	{
		return CreateApiError(BackendErrorCode, EDBANetworkErrorCategory::Conflict, NSLOCTEXT("DBAFrontendError", "CharacterNameTaken", "该角色名已被使用，请更换后重试。"), HttpStatusCode, false);
	}
	if (BackendErrorCode == TEXT("character.appearance_invalid"))
	{
		return CreateApiError(BackendErrorCode, EDBANetworkErrorCategory::Validation, NSLOCTEXT("DBAFrontendError", "AppearanceInvalid", "角色外观数据无效，请重新选择。"), HttpStatusCode, false);
	}
	if (BackendErrorCode == TEXT("session.ticket_invalid"))
	{
		return CreateApiError(BackendErrorCode, EDBANetworkErrorCategory::Authentication, NSLOCTEXT("DBAFrontendError", "TicketInvalid", "登录状态已失效，请重新登录。"), HttpStatusCode, false);
	}

	switch (HttpStatusCode)
	{
	case 0:
		return CreateApiError(BackendErrorCode.IsNone() ? TEXT("network.connectivity") : BackendErrorCode, EDBANetworkErrorCategory::Connectivity, NSLOCTEXT("DBAFrontendError", "Connectivity", "无法连接服务，请检查网络后重试。"), HttpStatusCode, true);
	case 400:
		return CreateApiError(BackendErrorCode.IsNone() ? TEXT("request.invalid") : BackendErrorCode, EDBANetworkErrorCategory::Validation, NSLOCTEXT("DBAFrontendError", "Validation", "提交的信息不符合要求，请检查后重试。"), HttpStatusCode, false);
	case 401:
		return CreateApiError(BackendErrorCode.IsNone() ? TEXT("auth.required") : BackendErrorCode, EDBANetworkErrorCategory::Authentication, NSLOCTEXT("DBAFrontendError", "Authentication", "登录状态无效，请重新登录。"), HttpStatusCode, false);
	case 403:
		return CreateApiError(BackendErrorCode.IsNone() ? TEXT("auth.forbidden") : BackendErrorCode, EDBANetworkErrorCategory::Authorization, NSLOCTEXT("DBAFrontendError", "Authorization", "当前账号没有执行此操作的权限。"), HttpStatusCode, false);
	case 404:
		return CreateApiError(BackendErrorCode.IsNone() ? TEXT("resource.not_found") : BackendErrorCode, EDBANetworkErrorCategory::NotFound, NSLOCTEXT("DBAFrontendError", "NotFound", "请求的资源不存在或已失效。"), HttpStatusCode, false);
	case 409:
		return CreateApiError(BackendErrorCode.IsNone() ? TEXT("request.conflict") : BackendErrorCode, EDBANetworkErrorCategory::Conflict, NSLOCTEXT("DBAFrontendError", "Conflict", "当前操作与服务器状态冲突，请刷新后重试。"), HttpStatusCode, true);
	case 429:
		return CreateApiError(BackendErrorCode.IsNone() ? TEXT("request.rate_limited") : BackendErrorCode, EDBANetworkErrorCategory::RateLimited, NSLOCTEXT("DBAFrontendError", "RateLimited", "请求过于频繁，请稍后重试。"), HttpStatusCode, true);
	case 408:
	case 504:
		return CreateApiError(BackendErrorCode.IsNone() ? TEXT("network.timeout") : BackendErrorCode, EDBANetworkErrorCategory::Timeout, NSLOCTEXT("DBAFrontendError", "Timeout", "请求超时，请稍后重试。"), HttpStatusCode, true);
	case 502:
	case 503:
		return CreateApiError(BackendErrorCode.IsNone() ? TEXT("service.unavailable") : BackendErrorCode, EDBANetworkErrorCategory::ServiceUnavailable, NSLOCTEXT("DBAFrontendError", "ServiceUnavailable", "服务暂不可用，请稍后重试。"), HttpStatusCode, true);
	default:
		return CreateApiError(BackendErrorCode.IsNone() ? TEXT("service.unknown") : BackendErrorCode, EDBANetworkErrorCategory::Unknown, NSLOCTEXT("DBAFrontendError", "Unknown", "操作未完成，请稍后重试。"), HttpStatusCode, true);
	}
}

FDBAApiError UDBAFrontendErrorMapper::FromLegacyMessage(const FString& LegacyMessage)
{
	// 旧调用方只能得到安全的通用文案；远端原文不再被转交给新的 UI 事件。
	return CreateApiError(TEXT("legacy.unclassified"), EDBANetworkErrorCategory::Unknown, NSLOCTEXT("DBAFrontendError", "Legacy", "操作未完成，请稍后重试。"), 0, true);
}
