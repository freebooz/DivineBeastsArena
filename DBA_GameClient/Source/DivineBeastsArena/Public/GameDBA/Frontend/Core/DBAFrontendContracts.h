// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Types/DBACommonEnums.h"
#include "DBAFrontendContracts.generated.h"

/** 前台全链路的唯一状态契约。旧登录状态只能作为该枚举的兼容投影，不能再承担状态机职责。 */
UENUM(BlueprintType)
enum class EDBAFrontendState : uint8
{
	Bootstrapping UMETA(DisplayName = "引导初始化"),
	Startup UMETA(DisplayName = "启动准备"),
	AutoLogin UMETA(DisplayName = "自动登录"),
	Login UMETA(DisplayName = "登录"),
	Register UMETA(DisplayName = "注册"),
	ServerSelect UMETA(DisplayName = "选择服务器"),
	CharacterRosterLoading UMETA(DisplayName = "加载角色列表"),
	CharacterSelect UMETA(DisplayName = "选择角色"),
	CharacterCreate_Zodiac UMETA(DisplayName = "创建角色：生肖与外观"),
	CharacterCreate_Element UMETA(DisplayName = "创建角色：元素"),
	CharacterCreate_FiveCamp UMETA(DisplayName = "创建角色：五营"),
	CharacterCreate_Confirm UMETA(DisplayName = "创建角色：确认名称"),
	EnteringWorld UMETA(DisplayName = "进入游戏"),
	RecoverableError UMETA(DisplayName = "可恢复错误"),
	FatalError UMETA(DisplayName = "不可恢复错误")
};

/** 所有异步前台操作的统一生命周期。 */
UENUM(BlueprintType)
enum class EDBAAsyncOperationState : uint8
{
	Idle UMETA(DisplayName = "空闲"),
	InProgress UMETA(DisplayName = "进行中"),
	Succeeded UMETA(DisplayName = "成功"),
	Failed UMETA(DisplayName = "失败"),
	TimedOut UMETA(DisplayName = "超时"),
	Cancelled UMETA(DisplayName = "已取消")
};

/** 网络/API 故障类别。Widget 只能基于该分类与结构化 ErrorCode 决定展示，不得解析远端英文文本。 */
UENUM(BlueprintType)
enum class EDBANetworkErrorCategory : uint8
{
	None UMETA(DisplayName = "无"),
	Cancelled UMETA(DisplayName = "已取消"),
	Timeout UMETA(DisplayName = "超时"),
	Connectivity UMETA(DisplayName = "连接失败"),
	Authentication UMETA(DisplayName = "认证失败"),
	Authorization UMETA(DisplayName = "无权限"),
	Validation UMETA(DisplayName = "请求校验失败"),
	NotFound UMETA(DisplayName = "资源不存在"),
	Conflict UMETA(DisplayName = "冲突"),
	RateLimited UMETA(DisplayName = "请求受限"),
	ServiceUnavailable UMETA(DisplayName = "服务不可用"),
	Protocol UMETA(DisplayName = "协议错误"),
	Unknown UMETA(DisplayName = "未知"),
};

/**
 * 可安全交给 UI 的 API 错误。该结构不保存响应原文、账号口令、会话令牌或进服 Ticket。
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAApiError
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Frontend|Error")
	FName ErrorCode = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Frontend|Error")
	EDBANetworkErrorCategory Category = EDBANetworkErrorCategory::None;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Frontend|Error")
	FText UserMessage;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Frontend|Error")
	int32 HttpStatusCode = 0;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Frontend|Error")
	bool bCanRetry = false;

	bool IsError() const { return !ErrorCode.IsNone(); }
};

/**
 * 前台跨 Screen 会话上下文。只含业务标识与角色创建草稿，严禁在这里保存 Token、Password、RefreshToken 或 GameTicket。
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAFrontendSessionContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Frontend|Session")
	FString AccountId;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Frontend|Session")
	FString ServerId;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Frontend|Session")
	FString SelectedCharacterId;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Frontend|Session")
	EDBAFrontendState ClientSessionState = EDBAFrontendState::Bootstrapping;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Frontend|Session")
	EDBAZodiac Zodiac = EDBAZodiac::None;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Frontend|Session")
	EDBAElement Element = EDBAElement::None;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Frontend|Session")
	EDBAFiveCamp FiveCamp = EDBAFiveCamp::None;

	void ResetCharacterDraft()
	{
		SelectedCharacterId.Reset();
		Zodiac = EDBAZodiac::None;
		Element = EDBAElement::None;
		FiveCamp = EDBAFiveCamp::None;
	}
};
