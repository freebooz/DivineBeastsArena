// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Core/DBAErrorCodes.h"

FString DBAErrorCodeHelpers::ToString(EDBAErrorCode ErrorCode)
{
	switch (ErrorCode)
	{
	case EDBAErrorCode::None:
		return TEXT("无错误");
	case EDBAErrorCode::InvalidData:
		return TEXT("数据无效");
	case EDBAErrorCode::InvalidState:
		return TEXT("状态无效");
	case EDBAErrorCode::ServiceUnavailable:
		return TEXT("服务不可用");
	case EDBAErrorCode::NetworkFailure:
		return TEXT("网络失败");
	case EDBAErrorCode::Timeout:
		return TEXT("超时");
	case EDBAErrorCode::Unauthorized:
		return TEXT("未授权");
	case EDBAErrorCode::NotFound:
		return TEXT("未找到");
	case EDBAErrorCode::DuplicateRequest:
		return TEXT("重复请求");
	case EDBAErrorCode::VersionMismatch:
		return TEXT("版本不匹配");
	case EDBAErrorCode::TravelFailure:
		return TEXT("Travel失败");
	case EDBAErrorCode::MatchFailure:
		return TEXT("匹配失败");
	case EDBAErrorCode::SaveFailure:
		return TEXT("保存失败");
	case EDBAErrorCode::LoadFailure:
		return TEXT("加载失败");
	case EDBAErrorCode::ExternalServiceDisabled:
		return TEXT("外部服务已禁用");
	case EDBAErrorCode::ExternalServiceCircuitOpen:
		return TEXT("外部服务熔断");
	case EDBAErrorCode::TelemetryDropped:
		return TEXT("遥测数据丢弃");
	case EDBAErrorCode::Unknown:
		return TEXT("未知错误");
	default:
		return TEXT("未定义错误");
	}
}

bool DBAErrorCodeHelpers::IsExternalServiceError(EDBAErrorCode ErrorCode)
{
	return ErrorCode == EDBAErrorCode::ExternalServiceDisabled
		|| ErrorCode == EDBAErrorCode::ExternalServiceCircuitOpen
		|| ErrorCode == EDBAErrorCode::TelemetryDropped;
}

bool DBAErrorCodeHelpers::IsFatalError(EDBAErrorCode ErrorCode)
{
	return ErrorCode == EDBAErrorCode::InvalidData
		|| ErrorCode == EDBAErrorCode::VersionMismatch
		|| ErrorCode == EDBAErrorCode::TravelFailure;
}