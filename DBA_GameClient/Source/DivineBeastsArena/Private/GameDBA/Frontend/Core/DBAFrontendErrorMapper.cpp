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
	const FDBAApiError CharacterCreateError = FromCharacterCreateErrorCode(BackendErrorCode, HttpStatusCode);
	if (CharacterCreateError.IsError())
	{
		return CharacterCreateError;
	}
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

FDBAApiError UDBAFrontendErrorMapper::FromCharacterCreateErrorCode(const FName BackendErrorCode, const int32 HttpStatusCode)
{
	// 同时兼容目标协议码和当前 CharacterService 已落地的 CHARACTER_* / APPEARANCE_* 码，
	// 让 UI 始终以结构化 ErrorCode 决定回退到名字、外观或通用重试状态。
	if (BackendErrorCode == TEXT("NAME_EXISTS") || BackendErrorCode == TEXT("CHARACTER_NAME_DUPLICATE"))
	{
		return CreateApiError(BackendErrorCode, EDBANetworkErrorCategory::Conflict, NSLOCTEXT("DBAFrontendError", "CreateNameExists", "该角色名已被使用，请更换名称后重试。"), HttpStatusCode, false);
	}
	if (BackendErrorCode == TEXT("INVALID_NAME") || BackendErrorCode == TEXT("CHARACTER_NAME_INVALID"))
	{
		return CreateApiError(BackendErrorCode, EDBANetworkErrorCategory::Validation, NSLOCTEXT("DBAFrontendError", "CreateInvalidName", "角色名不符合规则，请修改后重试。"), HttpStatusCode, false);
	}
	if (BackendErrorCode == TEXT("SLOT_LIMIT") || BackendErrorCode == TEXT("CHARACTER_SLOT_LIMIT"))
	{
		return CreateApiError(BackendErrorCode, EDBANetworkErrorCategory::Conflict, NSLOCTEXT("DBAFrontendError", "CreateSlotLimit", "当前区服角色槽位已满，请删除角色或选择其他区服。"), HttpStatusCode, false);
	}
	if (BackendErrorCode == TEXT("INVALID_APPEARANCE") || BackendErrorCode == TEXT("APPEARANCE_OPTION_INVALID"))
	{
		return CreateApiError(BackendErrorCode, EDBANetworkErrorCategory::Validation, NSLOCTEXT("DBAFrontendError", "CreateInvalidAppearance", "所选外观不适用于当前生肖，请返回外观步骤调整。"), HttpStatusCode, false);
	}
	if (BackendErrorCode == TEXT("INVALID_BUILD") || BackendErrorCode == TEXT("CHARACTER_BUILD_INVALID"))
	{
		return CreateApiError(BackendErrorCode, EDBANetworkErrorCategory::Validation, NSLOCTEXT("DBAFrontendError", "CreateInvalidBuild", "生肖、元素或五营组合无效，请返回对应步骤调整。"), HttpStatusCode, false);
	}
	if (BackendErrorCode == TEXT("SERVER_MAINTENANCE"))
	{
		return CreateApiError(BackendErrorCode, EDBANetworkErrorCategory::ServiceUnavailable, NSLOCTEXT("DBAFrontendError", "CreateServerMaintenance", "当前区服正在维护，暂时无法创建角色。"), HttpStatusCode, true);
	}
	return FDBAApiError();
}

FDBAApiError UDBAFrontendErrorMapper::FromLegacyMessage(const FString& LegacyMessage)
{
	// 旧调用方只能得到安全的通用文案；远端原文不再被转交给新的 UI 事件。
	return CreateApiError(TEXT("legacy.unclassified"), EDBANetworkErrorCategory::Unknown, NSLOCTEXT("DBAFrontendError", "Legacy", "操作未完成，请稍后重试。"), 0, true);
}
