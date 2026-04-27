// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/Types/DBACommonTypes.h"
#include "Common/Account/DBAAccountTypes.h"
#include "DBAMatchSessionTypes.generated.h"

/**
 * 匹配会话 ID
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAMatchSessionId
{
	GENERATED_BODY()

	/**
	 * 匹配会话 ID 字符串
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchSession")
	FString Id;

	FDBAMatchSessionId()
		: Id(TEXT(""))
	{
	}

	explicit FDBAMatchSessionId(const FString& InId)
		: Id(InId)
	{
	}

	bool IsValid() const
	{
		return !Id.IsEmpty();
	}

	bool operator==(const FDBAMatchSessionId& Other) const
	{
		return Id == Other.Id;
	}

	bool operator!=(const FDBAMatchSessionId& Other) const
	{
		return !(*this == Other);
	}

	friend uint32 GetTypeHash(const FDBAMatchSessionId& SessionId)
	{
		return GetTypeHash(SessionId.Id);
	}

	FString ToString() const
	{
		return Id;
	}
};

/**
 * 匹配会话状态
 */
UENUM(BlueprintType)
enum class EDBAMatchSessionState : uint8
{
	/** 未定义 */
	None = 0 UMETA(DisplayName = "未定义"),

	/** 已创建 */
	Created UMETA(DisplayName = "已创建"),

	/** 准备确认中 */
	ReadyCheck UMETA(DisplayName = "准备确认中"),

	/** 加载中 */
	Loading UMETA(DisplayName = "加载中"),

	/** 进行中 */
	InProgress UMETA(DisplayName = "进行中"),

	/** 已完成 */
	Completed UMETA(DisplayName = "已完成"),

	/** 已取消 */
	Cancelled UMETA(DisplayName = "已取消")
};

/**
 * 玩家匹配信息
 * 包含玩家在匹配中的所有必要信息
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAPlayerMatchInfo
{
	GENERATED_BODY()

	/** 账户 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchSession")
	FDBAAccountId AccountId;

	/** 显示名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchSession")
	FString DisplayName;

	/** 队伍 ID（0 或 1） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchSession")
	int32 TeamId = 0;

	/** 选择的生肖 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchSession")
	EDBAZodiac SelectedZodiac = EDBAZodiac::None;

	/** 选择的自然元素之力 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchSession")
	EDBAElement SelectedElement = EDBAElement::None;

	/** 选择的五大阵营 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchSession")
	EDBAFiveCamp SelectedFiveCamp = EDBAFiveCamp::None;

	/** 是否已准备 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchSession")
	bool bIsReady = false;

	FDBAPlayerMatchInfo()
	{
	}

	bool IsValid() const
	{
		return AccountId.IsValid();
	}
};

/**
 * 匹配会话信息
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAMatchSessionInfo
{
	GENERATED_BODY()

	/** 匹配会话 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchSession")
	FDBAMatchSessionId SessionId;

	/** 会话状态 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchSession")
	EDBAMatchSessionState State = EDBAMatchSessionState::None;

	/** 玩家列表 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchSession")
	TArray<FDBAPlayerMatchInfo> Players;

	/** 地图名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchSession")
	FString MapName;

	/** 游戏模式 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchSession")
	FString GameMode;

	/** Dedicated Server 地址 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchSession")
	FString ServerAddress;

	/** Dedicated Server 端口 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchSession")
	int32 ServerPort = 0;

	/** 创建时间（Unix 时间戳） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchSession")
	int64 CreateTime = 0;

	FDBAMatchSessionInfo()
	{
	}

	bool IsValid() const
	{
		return SessionId.IsValid() && Players.Num() > 0;
	}

	FDBAPlayerMatchInfo* FindPlayer(const FDBAAccountId& AccountId)
	{
		return Players.FindByPredicate([&AccountId](const FDBAPlayerMatchInfo& Player)
		{
			return Player.AccountId == AccountId;
		});
	}
};

/**
 * 匹配找到通知
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAMatchFoundNotification
{
	GENERATED_BODY()

	/** 匹配会话信息 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchSession")
	FDBAMatchSessionInfo SessionInfo;

	/** 准备确认超时时间（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchSession")
	int32 ReadyCheckTimeoutSeconds = 30;

	/** 通知时间（Unix 时间戳） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MatchSession")
	int64 NotificationTime = 0;

	FDBAMatchFoundNotification()
	{
	}

	bool IsValid() const
	{
		return SessionInfo.IsValid();
	}
};

/**
 * 匹配会话服务回调委托
 */
DECLARE_DELEGATE_OneParam(FDBAOnMatchFound, const FDBAMatchFoundNotification& /* Notification */);
DECLARE_DELEGATE_OneParam(FDBAOnMatchSessionUpdated, const FDBAMatchSessionInfo& /* SessionInfo */);
DECLARE_DELEGATE(FDBAOnMatchCancelled);
