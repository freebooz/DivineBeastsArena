// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DBAErrorCodes.generated.h"

/**
 * 错误码枚举
 * 用于统一错误处理、UI 错误提示映射、日志记录、调试追踪
 * 所有错误码必须有详细中文注释，说明触发场景、影响范围、建议处理方式
 */
UENUM(BlueprintType)
enum class EDBAErrorCode : uint8
{
	/** 无错误，操作成功 */
	None = 0 UMETA(DisplayName = "无错误"),

	/** 数据无效：DataTable 行缺失、字段为空、引用资源不存在 */
	InvalidData UMETA(DisplayName = "数据无效"),

	/** 状态无效：当前游戏状态不允许执行此操作，如未登录时尝试匹配 */
	InvalidState UMETA(DisplayName = "状态无效"),

	/** 服务不可用：Dedicated Server 未响应、Session 创建失败 */
	ServiceUnavailable UMETA(DisplayName = "服务不可用"),

	/** 网络失败：连接超时、RPC 失败、复制中断 */
	NetworkFailure UMETA(DisplayName = "网络失败"),

	/** 超时：匹配超时、准备检查超时、Travel 超时 */
	Timeout UMETA(DisplayName = "超时"),

	/** 未授权：账户未登录、Token 过期、权限不足 */
	Unauthorized UMETA(DisplayName = "未授权"),

	/** 未找到：角色不存在、房间不存在、资源未加载 */
	NotFound UMETA(DisplayName = "未找到"),

	/** 重复请求：重复加入队列、重复准备、重复选择英雄 */
	DuplicateRequest UMETA(DisplayName = "重复请求"),

	/** 版本不匹配：客户端版本与服务端版本不一致 */
	VersionMismatch UMETA(DisplayName = "版本不匹配"),

	/** Travel 失败：关卡切换失败、地图加载失败 */
	TravelFailure UMETA(DisplayName = "Travel失败"),

	/** 匹配失败：队伍人数不足、匹配规则冲突、房间已满 */
	MatchFailure UMETA(DisplayName = "匹配失败"),

	/** 保存失败：SaveGame 写入失败、Profile 序列化失败 */
	SaveFailure UMETA(DisplayName = "保存失败"),

	/** 加载失败：SaveGame 读取失败、Profile 反序列化失败 */
	LoadFailure UMETA(DisplayName = "加载失败"),

	/** 外部服务已禁用：Monitoring / GameOps 功能已关闭，不影响游戏核心逻辑 */
	ExternalServiceDisabled UMETA(DisplayName = "外部服务已禁用"),

	/** 外部服务熔断：Monitoring / GameOps 连续失败触发熔断，不影响游戏核心逻辑 */
	ExternalServiceCircuitOpen UMETA(DisplayName = "外部服务熔断"),

	/** 遥测数据丢弃：Monitoring 上报队列满或超时，不影响游戏核心逻辑 */
	TelemetryDropped UMETA(DisplayName = "遥测数据丢弃"),

	/** 未知错误：未分类的异常情况 */
	Unknown UMETA(DisplayName = "未知错误")
};

/**
 * 错误码辅助函数
 */
namespace DBAErrorCodeHelpers
{
	/**
	 * 将错误码转换为可读字符串
	 * @param ErrorCode 错误码
	 * @return 错误码对应的中文描述
	 */
	DIVINEBEASTSARENA_API FString ToString(EDBAErrorCode ErrorCode);

	/**
	 * 判断错误码是否为外部服务相关错误（不影响游戏核心逻辑）
	 * @param ErrorCode 错误码
	 * @return 是否为外部服务错误
	 */
	DIVINEBEASTSARENA_API bool IsExternalServiceError(EDBAErrorCode ErrorCode);

	/**
	 * 判断错误码是否为致命错误（需要中断当前流程）
	 * @param ErrorCode 错误码
	 * @return 是否为致命错误
	 */
	DIVINEBEASTSARENA_API bool IsFatalError(EDBAErrorCode ErrorCode);
}