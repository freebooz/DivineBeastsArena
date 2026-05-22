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
#include "GameCore/Types/DBACommonTypes.h"
#include "DBAAccountTypes.generated.h"

/**
 * 账户登录类型
 */
UENUM(BlueprintType)
enum class EDBALoginType : uint8
{
	/** 未登录 */
	None = 0 UMETA(DisplayName = "未登录"),

	/** 游客登录 */
	Guest UMETA(DisplayName = "游客登录"),

	/** 邮箱登录 */
	Email UMETA(DisplayName = "邮箱登录"),

	/** 第三方登录（预留） */
	ThirdParty UMETA(DisplayName = "第三方登录")
};

/**
 * 账户状态
 */
UENUM(BlueprintType)
enum class EDBAAccountStatus : uint8
{
	/** 正常 */
	Normal = 0 UMETA(DisplayName = "正常"),

	/** 封禁 */
	Banned UMETA(DisplayName = "封禁"),

	/** 冻结 */
	Frozen UMETA(DisplayName = "冻结"),

	/** 待验证 */
	PendingVerification UMETA(DisplayName = "待验证")
};

/**
 * 账户 ID
 * 全局唯一标识符
 */
USTRUCT(BlueprintType)
struct GAMECORE_API FDBAAccountId
{
	GENERATED_BODY()

	/**
	 * 账户 ID 字符串
	 * Guest 模式：本地生成 GUID
	 * 在线模式：服务端分配 ID
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Account")
	FString Id;

	FDBAAccountId()
		: Id(TEXT(""))
	{
	}

	explicit FDBAAccountId(const FString& InId)
		: Id(InId)
	{
	}

	bool IsValid() const
	{
		return !Id.IsEmpty();
	}

	bool operator==(const FDBAAccountId& Other) const
	{
		return Id == Other.Id;
	}

	bool operator!=(const FDBAAccountId& Other) const
	{
		return !(*this == Other);
	}

	friend uint32 GetTypeHash(const FDBAAccountId& AccountId)
	{
		return GetTypeHash(AccountId.Id);
	}

	FString ToString() const
	{
		return Id;
	}
};

/**
 * 角色 ID
 * 角色唯一标识符
 * 注意：角色 ID != 对局内 Zodiac 选择
 * 角色是账户下的持久化实体，对局内可以选择不同的 Zodiac / Element / FiveCamp
 */
USTRUCT(BlueprintType)
struct GAMECORE_API FDBACharacterId
{
	GENERATED_BODY()

	/**
	 * 角色 ID 字符串
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	FString Id;

	FDBACharacterId()
		: Id(TEXT(""))
	{
	}

	explicit FDBACharacterId(const FString& InId)
		: Id(InId)
	{
	}

	bool IsValid() const
	{
		return !Id.IsEmpty();
	}

	bool operator==(const FDBACharacterId& Other) const
	{
		return Id == Other.Id;
	}

	bool operator!=(const FDBACharacterId& Other) const
	{
		return !(*this == Other);
	}

	friend uint32 GetTypeHash(const FDBACharacterId& CharacterId)
	{
		return GetTypeHash(CharacterId.Id);
	}

	FString ToString() const
	{
		return Id;
	}
};

/**
 * 账户基础信息
 */
USTRUCT(BlueprintType)
struct GAMECORE_API FDBAAccountInfo
{
	GENERATED_BODY()

	/** 账户 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Account")
	FDBAAccountId AccountId;

	/** 显示名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Account")
	FString DisplayName;

	/** 登录类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Account")
	EDBALoginType LoginType = EDBALoginType::None;

	/** 账户状态 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Account")
	EDBAAccountStatus Status = EDBAAccountStatus::Normal;

	/** 账户等级 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Account")
	int32 Level = 1;

	/** 当前经验值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Account")
	int32 Experience = 0;

	/** 创建时间（Unix 时间戳） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Account")
	int64 CreateTime = 0;

	/** 最后登录时间（Unix 时间戳） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Account")
	int64 LastLoginTime = 0;

	FDBAAccountInfo()
	{
	}

	bool IsValid() const
	{
		return AccountId.IsValid() && !DisplayName.IsEmpty();
	}
};

/**
 * v4/v4.1 角色核心同步属性
 */
USTRUCT(BlueprintType)
struct GAMECORE_API FDBACharacterCoreAttributes
{
	GENERATED_BODY()

	/** 最大生命值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Core Attributes")
	float MaxHealth = 1800.0f;

	/** 攻击力 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Core Attributes")
	float AttackPower = 100.0f;

	/** 防御力 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Core Attributes")
	float Defense = 40.0f;

	/** 移动速度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Core Attributes")
	float MoveSpeed = 380.0f;

	/** 最大能量 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Core Attributes")
	float MaxEnergy = 100.0f;

	/** 能量回复 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Core Attributes")
	float EnergyRegen = 10.0f;

	/** 暴击率 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Core Attributes")
	float CriticalRate = 5.0f;

	/** 暴击倍率 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Core Attributes")
	float CriticalMultiplier = 200.0f;

	FDBACharacterCoreAttributes()
	{
	}
};

/**
 * 角色摘要信息
 * 用于角色选择界面显示
 */
USTRUCT(BlueprintType)
struct GAMECORE_API FDBACharacterSummary
{
	GENERATED_BODY()

	/** 角色 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	FDBACharacterId CharacterId;

	/** 角色名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	FString CharacterName;

	/** v4 创建选择：生肖 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Creation")
	EDBAZodiac Zodiac = EDBAZodiac::None;

	/** v4 创建选择：主元素（金木水火土） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Creation")
	EDBAElement PrimaryElement = EDBAElement::None;

	/** v4 创建选择：五大阵营 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Creation")
	EDBAFiveCamp FiveCamp = EDBAFiveCamp::None;

	/** 系统自动生成的固定技能组 ID，例如 Rat_Water */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Creation")
	FName FixedSkillGroupId = NAME_None;

	/** v4/v4.1 核心同步属性 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Core Attributes")
	FDBACharacterCoreAttributes CoreAttributes;

	/** 默认生肖 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	EDBAZodiac DefaultZodiac = EDBAZodiac::None;

	/** 默认自然元素之力 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	EDBAElement DefaultElement = EDBAElement::None;

	/** 默认五大阵营 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	EDBAFiveCamp DefaultFiveCamp = EDBAFiveCamp::None;

	/** 角色等级 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	int32 Level = 1;

	/** 创建时间（Unix 时间戳） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	int64 CreateTime = 0;

	/** 最后使用时间（Unix 时间戳） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	int64 LastUsedTime = 0;

	FDBACharacterSummary()
	{
	}

	bool IsValid() const
	{
		return CharacterId.IsValid() && !CharacterName.IsEmpty();
	}
};

/**
 * 角色详细信息
 * 包含角色的完整数据
 */
USTRUCT(BlueprintType)
struct GAMECORE_API FDBACharacterProfile
{
	GENERATED_BODY()

	/** 角色摘要 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	FDBACharacterSummary Summary;

	/** 总游戏场次 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	int32 TotalGames = 0;

	/** 胜利场次 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	int32 WinGames = 0;

	/** 总击杀数 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	int32 TotalKills = 0;

	/** 总死亡数 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	int32 TotalDeaths = 0;

	/** 总助攻数 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	int32 TotalAssists = 0;

	/** 自定义数据（JSON 格式） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	FString CustomData;

	FDBACharacterProfile()
	{
	}

	bool IsValid() const
	{
		return Summary.IsValid();
	}

	float GetWinRate() const
	{
		return TotalGames > 0 ? static_cast<float>(WinGames) / TotalGames : 0.0f;
	}

	float GetKDA() const
	{
		return TotalDeaths > 0 ? static_cast<float>(TotalKills + TotalAssists) / TotalDeaths : static_cast<float>(TotalKills + TotalAssists);
	}
};

/**
 * 登录请求
 */
USTRUCT(BlueprintType)
struct GAMECORE_API FDBALoginRequest
{
	GENERATED_BODY()

	/** 登录类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Account")
	EDBALoginType LoginType = EDBALoginType::Guest;

	/** 邮箱（Email 登录时使用） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Account")
	FString Email;

	/** 密码（Email 登录时使用） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Account")
	FString Password;

	/** 第三方 Token（ThirdParty 登录时使用） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Account")
	FString ThirdPartyToken;

	/** 设备 ID（用于 Guest 登录绑定设备） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Account")
	FString DeviceId;

	FDBALoginRequest()
	{
	}
};

/**
 * 登录响应
 */
USTRUCT(BlueprintType)
struct GAMECORE_API FDBALoginResponse
{
	GENERATED_BODY()

	/** 是否成功 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Account")
	bool bSuccess = false;

	/** 错误消息 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Account")
	FString ErrorMessage;

	/** 账户信息 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Account")
	FDBAAccountInfo AccountInfo;

	/** 会话 Token（用于后续请求验证） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Account")
	FString SessionToken;

	/** 刷新 Token（用于续期访问令牌） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Account")
	FString RefreshToken;

	/** 后端玩家 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Account")
	FString PlayerId;

	FDBALoginResponse()
	{
	}
};

/**
 * 角色创建请求
 */
USTRUCT(BlueprintType)
struct GAMECORE_API FDBACharacterCreateRequest
{
	GENERATED_BODY()

	/** 角色名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	FString CharacterName;

	/** 兼容字段：默认生肖 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	EDBAZodiac DefaultZodiac = EDBAZodiac::Rat;

	/** v4 创建选择：生肖 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Creation")
	EDBAZodiac Zodiac = EDBAZodiac::None;

	/** v4 创建选择：主元素（金木水火土） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Creation")
	EDBAElement PrimaryElement = EDBAElement::None;

	/** v4 创建选择：五大阵营；None 表示由系统默认分配 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Creation")
	EDBAFiveCamp FiveCamp = EDBAFiveCamp::None;

	/** 默认自然元素之力 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	EDBAElement DefaultElement = EDBAElement::None;

	/** 默认五大阵营 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	EDBAFiveCamp DefaultFiveCamp = EDBAFiveCamp::None;

	FDBACharacterCreateRequest()
	{
	}
};

/**
 * 角色创建响应
 */
USTRUCT(BlueprintType)
struct GAMECORE_API FDBACharacterCreateResponse
{
	GENERATED_BODY()

	/** 是否成功 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	bool bSuccess = false;

	/** 错误消息 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	FString ErrorMessage;

	/** 角色摘要 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	FDBACharacterSummary CharacterSummary;

	FDBACharacterCreateResponse()
	{
	}
};

/**
 * 账户服务回调委托
 */
DECLARE_DELEGATE_OneParam(FDBAOnLoginComplete, const FDBALoginResponse& /* Response */);
DECLARE_DELEGATE_OneParam(FDBAOnCharacterListLoaded, const TArray<FDBACharacterSummary>& /* Characters */);
DECLARE_DELEGATE_OneParam(FDBAOnCharacterProfileLoaded, const FDBACharacterProfile& /* Profile */);
DECLARE_DELEGATE_OneParam(FDBAOnCharacterCreated, const FDBACharacterCreateResponse& /* Response */);
DECLARE_DELEGATE_OneParam(FDBAOnCharacterDeleted, bool /* bSuccess */);
DECLARE_DELEGATE_OneParam(FDBAOnCharacterSelected, const FDBACharacterId& /* CharacterId */);
DECLARE_DELEGATE(FDBAOnLogoutComplete);
