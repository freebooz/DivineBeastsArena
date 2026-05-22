// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#pragma once

#include "CoreMinimal.h"
#include "GameCore/Party/DBAPartyTypes.h"
#include "DBAQueueTypes.generated.h"

/**
 * 队列 ID
 */
USTRUCT(BlueprintType)
struct GAMECORE_API FDBAQueueId
{
	GENERATED_BODY()

	/**
	 * 队列 ID 字符串
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Queue")
	FString Id;

	FDBAQueueId()
		: Id(TEXT(""))
	{
	}

	explicit FDBAQueueId(const FString& InId)
		: Id(InId)
	{
	}

	bool IsValid() const
	{
		return !Id.IsEmpty();
	}

	bool operator==(const FDBAQueueId& Other) const
	{
		return Id == Other.Id;
	}

	bool operator!=(const FDBAQueueId& Other) const
	{
		return !(*this == Other);
	}

	friend uint32 GetTypeHash(const FDBAQueueId& QueueId)
	{
		return GetTypeHash(QueueId.Id);
	}

	FString ToString() const
	{
		return Id;
	}
};

/**
 * 队列类型
 */
UENUM(BlueprintType)
enum class EDBAQueueType : uint8
{
	/** 未定义 */
	None = 0 UMETA(DisplayName = "未定义"),

	/** 快速匹配 */
	QuickMatch UMETA(DisplayName = "快速匹配"),

	/** 排位赛 */
	Ranked UMETA(DisplayName = "排位赛"),

	/** 自定义 */
	Custom UMETA(DisplayName = "自定义"),

	/** 练习模式 */
	Practice UMETA(DisplayName = "练习模式")
};

/**
 * 队列状态
 */
UENUM(BlueprintType)
enum class EDBAQueueState : uint8
{
	/** 未在队列 */
	NotInQueue = 0 UMETA(DisplayName = "未在队列"),

	/** 匹配中 */
	Searching UMETA(DisplayName = "匹配中"),

	/** 已找到匹配 */
	MatchFound UMETA(DisplayName = "已找到匹配"),

	/** 已取消 */
	Cancelled UMETA(DisplayName = "已取消"),

	/** 超时 */
	Timeout UMETA(DisplayName = "超时")
};

/**
 * 队列信息
 */
USTRUCT(BlueprintType)
struct GAMECORE_API FDBAQueueInfo
{
	GENERATED_BODY()

	/** 队列 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Queue")
	FDBAQueueId QueueId;

	/** 队列类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Queue")
	EDBAQueueType QueueType = EDBAQueueType::QuickMatch;

	/** 队伍 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Queue")
	FDBAPartyId PartyId;

	/** 队列状态 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Queue")
	EDBAQueueState State = EDBAQueueState::NotInQueue;

	/** 开始时间（Unix 时间戳） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Queue")
	int64 StartTime = 0;

	/** 预计等待时间（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Queue")
	int32 EstimatedWaitTime = 0;

	FDBAQueueInfo()
	{
	}

	bool IsValid() const
	{
		return QueueId.IsValid() && PartyId.IsValid();
	}

	int32 GetElapsedTime() const
	{
		return static_cast<int32>(FDateTime::UtcNow().ToUnixTimestamp() - StartTime);
	}
};

/**
 * 队列更新信息
 */
USTRUCT(BlueprintType)
struct GAMECORE_API FDBAQueueUpdate
{
	GENERATED_BODY()

	/** 队列信息 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Queue")
	FDBAQueueInfo QueueInfo;

	/** 当前队列人数 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Queue")
	int32 CurrentQueueSize = 0;

	/** 更新时间（Unix 时间戳） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Queue")
	int64 UpdateTime = 0;

	FDBAQueueUpdate()
	{
	}
};

/**
 * 队列服务回调委托
 */
DECLARE_DELEGATE_OneParam(FDBAOnQueueStarted, const FDBAQueueInfo& /* QueueInfo */);
DECLARE_DELEGATE(FDBAOnQueueCancelled);
DECLARE_DELEGATE_OneParam(FDBAOnQueueUpdated, const FDBAQueueUpdate& /* Update */);
DECLARE_DELEGATE_TwoParams(FDBAOnQueueOperationComplete, bool /* bSuccess */, const FString& /* ErrorMessage */);
