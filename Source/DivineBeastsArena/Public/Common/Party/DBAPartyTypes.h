// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/Account/DBAAccountTypes.h"
#include "DBAPartyTypes.generated.h"

/**
 * 队伍 ID
 * 全局唯一标识符
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAPartyId
{
	GENERATED_BODY()

	/**
	 * 队伍 ID 字符串
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	FString Id;

	FDBAPartyId()
		: Id(TEXT(""))
	{
	}

	explicit FDBAPartyId(const FString& InId)
		: Id(InId)
	{
	}

	bool IsValid() const
	{
		return !Id.IsEmpty();
	}

	bool operator==(const FDBAPartyId& Other) const
	{
		return Id == Other.Id;
	}

	bool operator!=(const FDBAPartyId& Other) const
	{
		return !(*this == Other);
	}

	friend uint32 GetTypeHash(const FDBAPartyId& PartyId)
	{
		return GetTypeHash(PartyId.Id);
	}

	FString ToString() const
	{
		return Id;
	}
};

/**
 * 队伍成员角色
 */
UENUM(BlueprintType)
enum class EDBAPartyMemberRole : uint8
{
	/** 队长 */
	Leader = 0 UMETA(DisplayName = "队长"),

	/** 普通成员 */
	Member UMETA(DisplayName = "普通成员")
};

/**
 * 队伍成员状态
 */
UENUM(BlueprintType)
enum class EDBAPartyMemberStatus : uint8
{
	/** 在线 */
	Online = 0 UMETA(DisplayName = "在线"),

	/** 离线 */
	Offline UMETA(DisplayName = "离线"),

	/** 准备中 */
	Ready UMETA(DisplayName = "准备中"),

	/** 未准备 */
	NotReady UMETA(DisplayName = "未准备")
};

/**
 * 队伍状态
 */
UENUM(BlueprintType)
enum class EDBAPartyState : uint8
{
	/** 空闲 */
	Idle = 0 UMETA(DisplayName = "空闲"),

	/** 匹配中 */
	InQueue UMETA(DisplayName = "匹配中"),

	/** 准备确认中 */
	InReadyCheck UMETA(DisplayName = "准备确认中"),

	/** 加载中 */
	Loading UMETA(DisplayName = "加载中"),

	/** 对局中 */
	InMatch UMETA(DisplayName = "对局中")
};

/**
 * 队伍成员信息
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAPartyMember
{
	GENERATED_BODY()

	/** 账户 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	FDBAAccountId AccountId;

	/** 显示名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	FString DisplayName;

	/** 角色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	EDBAPartyMemberRole Role = EDBAPartyMemberRole::Member;

	/** 状态 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	EDBAPartyMemberStatus Status = EDBAPartyMemberStatus::Online;

	/** 加入时间（Unix 时间戳） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	int64 JoinTime = 0;

	FDBAPartyMember()
	{
	}

	bool IsValid() const
	{
		return AccountId.IsValid() && !DisplayName.IsEmpty();
	}

	bool IsLeader() const
	{
		return Role == EDBAPartyMemberRole::Leader;
	}
};

/**
 * 队伍信息
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAPartyInfo
{
	GENERATED_BODY()

	/** 队伍 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	FDBAPartyId PartyId;

	/** 队长账户 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	FDBAAccountId LeaderAccountId;

	/** 成员列表 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	TArray<FDBAPartyMember> Members;

	/** 队伍状态 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	EDBAPartyState State = EDBAPartyState::Idle;

	/** 最大成员数 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	int32 MaxMembers = 5;

	/** 创建时间（Unix 时间戳） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	int64 CreateTime = 0;

	FDBAPartyInfo()
	{
	}

	bool IsValid() const
	{
		return PartyId.IsValid() && LeaderAccountId.IsValid() && Members.Num() > 0;
	}

	bool IsFull() const
	{
		return Members.Num() >= MaxMembers;
	}

	FDBAPartyMember* FindMember(const FDBAAccountId& AccountId)
	{
		return Members.FindByPredicate([&AccountId](const FDBAPartyMember& Member)
		{
			return Member.AccountId == AccountId;
		});
	}

	const FDBAPartyMember* FindMember(const FDBAAccountId& AccountId) const
	{
		return Members.FindByPredicate([&AccountId](const FDBAPartyMember& Member)
		{
			return Member.AccountId == AccountId;
		});
	}
};

/**
 * 队伍邀请信息
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAPartyInvite
{
	GENERATED_BODY()

	/** 队伍 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	FDBAPartyId PartyId;

	/** 邀请者账户 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	FDBAAccountId InviterAccountId;

	/** 邀请者显示名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	FString InviterDisplayName;

	/** 被邀请者账户 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	FDBAAccountId InviteeAccountId;

	/** 邀请时间（Unix 时间戳） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	int64 InviteTime = 0;

	/** 过期时间（Unix 时间戳） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Party")
	int64 ExpireTime = 0;

	FDBAPartyInvite()
	{
	}

	bool IsValid() const
	{
		return PartyId.IsValid() && InviterAccountId.IsValid() && InviteeAccountId.IsValid();
	}

	bool IsExpired() const
	{
		return FDateTime::UtcNow().ToUnixTimestamp() >= ExpireTime;
	}
};

/**
 * 队伍服务回调委托
 */
DECLARE_DELEGATE_OneParam(FDBAOnPartyCreated, const FDBAPartyInfo& /* PartyInfo */);
DECLARE_DELEGATE_OneParam(FDBAOnPartyJoined, const FDBAPartyInfo& /* PartyInfo */);
DECLARE_DELEGATE(FDBAOnPartyLeft);
DECLARE_DELEGATE_OneParam(FDBAOnPartyUpdated, const FDBAPartyInfo& /* PartyInfo */);
DECLARE_DELEGATE_OneParam(FDBAOnPartyInviteReceived, const FDBAPartyInvite& /* Invite */);
DECLARE_DELEGATE_TwoParams(FDBAOnPartyOperationComplete, bool /* bSuccess */, const FString& /* ErrorMessage */);
