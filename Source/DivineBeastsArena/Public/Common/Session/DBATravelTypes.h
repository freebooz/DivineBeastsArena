// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/Types/DBACommonTypes.h"
#include "Common/Account/DBAAccountTypes.h"
#include "Common/Session/DBAMatchSessionTypes.h"
#include "DBATravelTypes.generated.h"

/**
 * Travel 上下文
 * 包含 Travel 所需的所有信息
 *
 * 重要：
 * - Travel DTO 传递 Zodiac / Element / FiveCamp / FixedSkillGroup
 * - 客户端不能决定最终 Match / Travel 结果
 * - Dedicated Server 验证所有 Travel 参数
 * - Travel 失败时必须返回前台
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBATravelContext
{
	GENERATED_BODY()

	/** 匹配会话 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Travel")
	FDBAMatchSessionId MatchSessionId;

	/** 账户 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Travel")
	FDBAAccountId AccountId;

	/** 角色 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Travel")
	FDBACharacterId CharacterId;

	/** 队伍 ID（0 或 1） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Travel")
	int32 TeamId = 0;

	/** 选择的生肖 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Travel")
	EDBAZodiac SelectedZodiac = EDBAZodiac::None;

	/** 选择的自然元素之力 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Travel")
	EDBAElement SelectedElement = EDBAElement::None;

	/** 选择的五大阵营 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Travel")
	EDBAFiveCamp SelectedFiveCamp = EDBAFiveCamp::None;

	/** 目标地图名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Travel")
	FString MapName;

	/** 目标服务器地址 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Travel")
	FString ServerAddress;

	/** 目标服务器端口 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Travel")
	int32 ServerPort = 0;

	/** 会话 Token（用于服务端验证） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Travel")
	FString SessionToken;

	/** Travel 选项（URL 参数） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Travel")
	FString TravelOptions;

	FDBATravelContext()
	{
	}

	bool IsValid() const
	{
		return MatchSessionId.IsValid()
			&& AccountId.IsValid()
			&& !MapName.IsEmpty()
			&& !ServerAddress.IsEmpty()
			&& ServerPort > 0;
	}

	/**
	 * 构建 Travel URL
	 * 格式：ServerAddress:ServerPort/MapName?Options
	 */
	FString BuildTravelURL() const
	{
		FString URL = FString::Printf(TEXT("%s:%d/%s"), *ServerAddress, ServerPort, *MapName);
		if (!TravelOptions.IsEmpty())
		{
			URL += TEXT("?") + TravelOptions;
		}
		return URL;
	}

	/**
	 * 添加 Travel 选项
	 */
	void AddTravelOption(const FString& Key, const FString& Value)
	{
		if (!TravelOptions.IsEmpty())
		{
			TravelOptions += TEXT("&");
		}
		TravelOptions += FString::Printf(TEXT("%s=%s"), *Key, *Value);
	}
};

/**
 * Travel 状态
 */
UENUM(BlueprintType)
enum class EDBATravelState : uint8
{
	/** 未开始 */
	None = 0 UMETA(DisplayName = "未开始"),

	/** 准备中 */
	Preparing UMETA(DisplayName = "准备中"),

	/** Travel 中 */
	Travelling UMETA(DisplayName = "Travel 中"),

	/** 已到达 */
	Arrived UMETA(DisplayName = "已到达"),

	/** 失败 */
	Failed UMETA(DisplayName = "失败"),

	/** 已取消 */
	Cancelled UMETA(DisplayName = "已取消")
};

/**
 * Travel 失败原因
 */
UENUM(BlueprintType)
enum class EDBATravelFailureReason : uint8
{
	/** 未知 */
	Unknown = 0 UMETA(DisplayName = "未知"),

	/** 网络错误 */
	NetworkError UMETA(DisplayName = "网络错误"),

	/** 服务器不可达 */
	ServerUnreachable UMETA(DisplayName = "服务器不可达"),

	/** 服务器已满 */
	ServerFull UMETA(DisplayName = "服务器已满"),

	/** 验证失败 */
	AuthenticationFailed UMETA(DisplayName = "验证失败"),

	/** 超时 */
	Timeout UMETA(DisplayName = "超时"),

	/** 地图加载失败 */
	MapLoadFailed UMETA(DisplayName = "地图加载失败"),

	/** 会话无效 */
	InvalidSession UMETA(DisplayName = "会话无效"),

	/** 用户取消 */
	UserCancelled UMETA(DisplayName = "用户取消")
};

/**
 * Travel 结果
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBATravelResult
{
	GENERATED_BODY()

	/** 是否成功 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Travel")
	bool bSuccess = false;

	/** Travel 状态 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Travel")
	EDBATravelState State = EDBATravelState::None;

	/** 失败原因 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Travel")
	EDBATravelFailureReason FailureReason = EDBATravelFailureReason::Unknown;

	/** 错误消息 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Travel")
	FString ErrorMessage;

	FDBATravelResult()
	{
	}
};

/**
 * 返回前台请求
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAReturnToFrontendRequest
{
	GENERATED_BODY()

	/** 返回原因 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Travel")
	FString Reason;

	/** 是否显示错误消息 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Travel")
	bool bShowError = false;

	/** 错误消息 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Travel")
	FString ErrorMessage;

	FDBAReturnToFrontendRequest()
	{
	}
};

/**
 * Travel 服务回调委托
 */
DECLARE_DELEGATE_OneParam(FDBAOnTravelStarted, const FDBATravelContext& /* Context */);
DECLARE_DELEGATE_OneParam(FDBAOnTravelCompleted, const FDBATravelResult& /* Result */);
DECLARE_DELEGATE_OneParam(FDBAOnTravelFailed, const FDBATravelResult& /* Result */);
DECLARE_DELEGATE(FDBAOnReturnedToFrontend);
