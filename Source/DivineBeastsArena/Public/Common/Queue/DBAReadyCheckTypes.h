// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/Account/DBAAccountTypes.h"
#include "Common/Session/DBAMatchSessionTypes.h"
#include "DBAReadyCheckTypes.generated.h"

/**
 * 准备确认 ID
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAReadyCheckId
{
	GENERATED_BODY()

	/**
	 * 准备确认 ID 字符串
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReadyCheck")
	FString Id;

	FDBAReadyCheckId()
		: Id(TEXT(""))
	{
	}

	explicit FDBAReadyCheckId(const FString& InId)
		: Id(InId)
	{
	}

	bool IsValid() const
	{
		return !Id.IsEmpty();
	}

	bool operator==(const FDBAReadyCheckId& Other) const
	{
		return Id == Other.Id;
	}

	bool operator!=(const FDBAReadyCheckId& Other) const
	{
		return !(*this == Other);
	}

	friend uint32 GetTypeHash(const FDBAReadyCheckId& ReadyCheckId)
	{
		return GetTypeHash(ReadyCheckId.Id);
	}

	FString ToString() const
	{
		return Id;
	}
};

/**
 * 准备确认状态
 */
UENUM(BlueprintType)
enum class EDBAReadyCheckState : uint8
{
	/** 等待确认 */
	Pending = 0 UMETA(DisplayName = "等待确认"),

	/** 已确认 */
	Confirmed UMETA(DisplayName = "已确认"),

	/** 已拒绝 */
	Declined UMETA(DisplayName = "已拒绝"),

	/** 超时 */
	Timeout UMETA(DisplayName = "超时"),

	/** 已完成 */
	Completed UMETA(DisplayName = "已完成"),

	/** 已取消 */
	Cancelled UMETA(DisplayName = "已取消")
};

/**
 * 玩家准备状态
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAPlayerReadyStatus
{
	GENERATED_BODY()

	/** 账户 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReadyCheck")
	FDBAAccountId AccountId;

	/** 显示名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReadyCheck")
	FString DisplayName;

	/** 准备状态 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReadyCheck")
	EDBAReadyCheckState State = EDBAReadyCheckState::Pending;

	/** 响应时间（Unix 时间戳） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReadyCheck")
	int64 ResponseTime = 0;

	FDBAPlayerReadyStatus()
	{
	}

	bool IsValid() const
	{
		return AccountId.IsValid();
	}

	bool IsReady() const
	{
		return State == EDBAReadyCheckState::Confirmed;
	}
};

/**
 * 准备确认信息
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAReadyCheckInfo
{
	GENERATED_BODY()

	/** 准备确认 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReadyCheck")
	FDBAReadyCheckId ReadyCheckId;

	/** 匹配会话 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReadyCheck")
	FDBAMatchSessionId MatchSessionId;

	/** 玩家准备状态列表 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReadyCheck")
	TArray<FDBAPlayerReadyStatus> PlayerStatuses;

	/** 开始时间（Unix 时间戳） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReadyCheck")
	int64 StartTime = 0;

	/** 超时时间（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReadyCheck")
	int32 TimeoutSeconds = 30;

	/** 整体状态 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReadyCheck")
	EDBAReadyCheckState OverallState = EDBAReadyCheckState::Pending;

	FDBAReadyCheckInfo()
	{
	}

	bool IsValid() const
	{
		return ReadyCheckId.IsValid() && MatchSessionId.IsValid();
	}

	bool IsExpired() const
	{
		return FDateTime::UtcNow().ToUnixTimestamp() >= (StartTime + TimeoutSeconds);
	}

	int32 GetRemainingTime() const
	{
		const int64 Elapsed = FDateTime::UtcNow().ToUnixTimestamp() - StartTime;
		return FMath::Max(0, TimeoutSeconds - static_cast<int32>(Elapsed));
	}

	int32 GetReadyCount() const
	{
		int32 Count = 0;
		for (const FDBAPlayerReadyStatus& Status : PlayerStatuses)
		{
			if (Status.IsReady())
			{
				++Count;
			}
		}
		return Count;
	}

	bool AllPlayersReady() const
	{
		for (const FDBAPlayerReadyStatus& Status : PlayerStatuses)
		{
			if (!Status.IsReady())
			{
				return false;
			}
		}
		return PlayerStatuses.Num() > 0;
	}

	FDBAPlayerReadyStatus* FindPlayerStatus(const FDBAAccountId& AccountId)
	{
		return PlayerStatuses.FindByPredicate([&AccountId](const FDBAPlayerReadyStatus& Status)
		{
			return Status.AccountId == AccountId;
		});
	}
};

/**
 * 准备确认服务回调委托
 */
DECLARE_DELEGATE_OneParam(FDBAOnReadyCheckStarted, const FDBAReadyCheckInfo& /* ReadyCheckInfo */);
DECLARE_DELEGATE_OneParam(FDBAOnReadyCheckUpdated, const FDBAReadyCheckInfo& /* ReadyCheckInfo */);
DECLARE_DELEGATE_OneParam(FDBAOnReadyCheckCompleted, bool /* bSuccess */);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDBAOnReadyCheckCompletedMulticast, bool, bSuccess);
DECLARE_DELEGATE(FDBAOnReadyCheckCancelled);
DECLARE_MULTICAST_DELEGATE(FDBAOnReadyCheckTimeout);
