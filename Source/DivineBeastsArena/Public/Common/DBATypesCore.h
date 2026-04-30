// Copyright Freebooz Games, Inc. All Rights Reserved.
// 神兽竞技场 - 核心类型定义

#pragma once

#include "CoreMinimal.h"
#include "DBAEnumsCore.h"
#include "DBATypesCore.generated.h"

/**
 * FDBARowId
 * 数据表行 ID 轻量包装
 * 用于类型安全的数据表行引用
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBARowId
{
	GENERATED_BODY()

	/** 行名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Data")
	FName RowName;

	FDBARowId() : RowName(NAME_None) {}
	explicit FDBARowId(FName InRowName) : RowName(InRowName) {}

	bool IsValid() const { return !RowName.IsNone(); }
	bool operator==(const FDBARowId& Other) const { return RowName == Other.RowName; }
	bool operator!=(const FDBARowId& Other) const { return RowName != Other.RowName; }
	friend uint32 GetTypeHash(const FDBARowId& Id) { return GetTypeHash(Id.RowName); }
};

/**
 * FDBASkillId
 * 技能 ID 轻量包装
 * 用于技能唯一标识
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBASkillId
{
	GENERATED_BODY()

	/** 技能 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Skill")
	FName SkillName;

	FDBASkillId() : SkillName(NAME_None) {}
	explicit FDBASkillId(FName InSkillName) : SkillName(InSkillName) {}

	bool IsValid() const { return !SkillName.IsNone(); }
	bool operator==(const FDBASkillId& Other) const { return SkillName == Other.SkillName; }
	bool operator!=(const FDBASkillId& Other) const { return SkillName != Other.SkillName; }
	friend uint32 GetTypeHash(const FDBASkillId& Id) { return GetTypeHash(Id.SkillName); }
};

/**
 * FDBAAssetId
 * 资产 ID 轻量包装
 * 用于资产引用、异步加载
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAAssetId
{
	GENERATED_BODY()

	/** 资产路径 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Asset")
	FSoftObjectPath AssetPath;

	FDBAAssetId() {}
	explicit FDBAAssetId(const FSoftObjectPath& InPath) : AssetPath(InPath) {}

	bool IsValid() const { return AssetPath.IsValid(); }
	bool operator==(const FDBAAssetId& Other) const { return AssetPath == Other.AssetPath; }
	bool operator!=(const FDBAAssetId& Other) const { return AssetPath != Other.AssetPath; }
	friend uint32 GetTypeHash(const FDBAAssetId& Id) { return GetTypeHash(Id.AssetPath); }
};

/**
 * FDBAServiceEndpointId
 * 外部服务端点 ID
 * 用于可选的 Monitoring / GameOps 客户端
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAServiceEndpointId
{
	GENERATED_BODY()

	/** 端点名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|External")
	FString EndpointName;

	FDBAServiceEndpointId() {}
	explicit FDBAServiceEndpointId(const FString& InName) : EndpointName(InName) {}

	bool IsValid() const { return !EndpointName.IsEmpty(); }
	bool operator==(const FDBAServiceEndpointId& Other) const { return EndpointName == Other.EndpointName; }
	bool operator!=(const FDBAServiceEndpointId& Other) const { return EndpointName != Other.EndpointName; }
	friend uint32 GetTypeHash(const FDBAServiceEndpointId& Id) { return GetTypeHash(Id.EndpointName); }
};

/**
 * FDBARequestId
 * 请求 ID
 * 用于外部服务请求追踪
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBARequestId
{
	GENERATED_BODY()

	/** 请求 GUID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|External")
	FGuid RequestGuid;

	FDBARequestId() : RequestGuid(FGuid::NewGuid()) {}
	explicit FDBARequestId(const FGuid& InGuid) : RequestGuid(InGuid) {}

	bool IsValid() const { return RequestGuid.IsValid(); }
	bool operator==(const FDBARequestId& Other) const { return RequestGuid == Other.RequestGuid; }
	bool operator!=(const FDBARequestId& Other) const { return RequestGuid != Other.RequestGuid; }
	friend uint32 GetTypeHash(const FDBARequestId& Id) { return GetTypeHash(Id.RequestGuid); }
};

/**
 * FDBAIdempotencyKey
 * 幂等性键
 * 用于外部服务请求去重
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAIdempotencyKey
{
	GENERATED_BODY()

	/** 幂等性键字符串 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|External")
	FString KeyString;

	FDBAIdempotencyKey() {}
	explicit FDBAIdempotencyKey(const FString& InKey) : KeyString(InKey) {}

	bool IsValid() const { return !KeyString.IsEmpty(); }
	bool operator==(const FDBAIdempotencyKey& Other) const { return KeyString == Other.KeyString; }
	bool operator!=(const FDBAIdempotencyKey& Other) const { return KeyString != Other.KeyString; }
	friend uint32 GetTypeHash(const FDBAIdempotencyKey& Key) { return GetTypeHash(Key.KeyString); }
};

/**
 * FDBASchemaVersion
 * 数据模式版本
 * 用于数据兼容性检查
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBASchemaVersion
{
	GENERATED_BODY()

	/** 主版本号 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Version")
	int32 Major = 0;

	/** 次版本号 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Version")
	int32 Minor = 0;

	/** 修订版本号 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Version")
	int32 Patch = 0;

	FDBASchemaVersion() {}
	FDBASchemaVersion(int32 InMajor, int32 InMinor, int32 InPatch)
		: Major(InMajor), Minor(InMinor), Patch(InPatch) {}

	bool IsValid() const { return Major >= 0 && Minor >= 0 && Patch >= 0; }

	FString ToString() const
	{
		return FString::Printf(TEXT("%d.%d.%d"), Major, Minor, Patch);
	}

	bool operator==(const FDBASchemaVersion& Other) const
	{
		return Major == Other.Major && Minor == Other.Minor && Patch == Other.Patch;
	}

	bool operator!=(const FDBASchemaVersion& Other) const
	{
		return !(*this == Other);
	}

	bool operator<(const FDBASchemaVersion& Other) const
	{
		if (Major != Other.Major) return Major < Other.Major;
		if (Minor != Other.Minor) return Minor < Other.Minor;
		return Patch < Other.Patch;
	}

	bool operator>(const FDBASchemaVersion& Other) const
	{
		return Other < *this;
	}

	bool operator<=(const FDBASchemaVersion& Other) const
	{
		return !(Other < *this);
	}

	bool operator>=(const FDBASchemaVersion& Other) const
	{
		return !(*this < Other);
	}
};

/**
 * FDBADataVersion
 * 数据版本
 * 用于缓存失效、配置更新检测
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBADataVersion
{
	GENERATED_BODY()

	/** 版本值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Version")
	int32 Version = 0;

	FDBADataVersion() : Version(0) {}
	explicit FDBADataVersion(int32 InVersion) : Version(InVersion) {}

	bool IsValid() const { return Version >= 0; }
	bool operator==(const FDBADataVersion& Other) const { return Version == Other.Version; }
	bool operator!=(const FDBADataVersion& Other) const { return Version != Other.Version; }
	bool operator<(const FDBADataVersion& Other) const { return Version < Other.Version; }
	bool operator>(const FDBADataVersion& Other) const { return Version > Other.Version; }
};

/**
 * FDBAElementCounterInfo
 * 元素克制信息
 * 用于伤害计算、UI 提示
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAElementCounterInfo
{
	GENERATED_BODY()

	/** 攻击方元素 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Element")
	EDBAElementType AttackerElement = EDBAElementType::None;

	/** 防御方元素 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Element")
	EDBAElementType DefenderElement = EDBAElementType::None;

	/** 克制结果 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Element")
	EDBAElementCounterResult CounterResult = EDBAElementCounterResult::None;

	/** 倍率 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Element")
	float Multiplier = 1.0f;

	FDBAElementCounterInfo() {}
};

/**
 * FDBAResonanceInfo
 * 元素共鸣信息
 * 用于共鸣系统计算
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAResonanceInfo
{
	GENERATED_BODY()

	/** 元素类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Resonance")
	EDBAElementType Element = EDBAElementType::None;

	/** 同元素技能数量 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Resonance")
	int32 SameElementSkillCount = 0;

	/** 共鸣等级 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Resonance")
	EDBAResonanceLevel ResonanceLevel = EDBAResonanceLevel::None;

	/** 控制时间加成（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Resonance")
	float CCDurationBonus = 0.0f;

	/** 护盾值加成（百分比） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Resonance")
	float ShieldBonus = 0.0f;

	FDBAResonanceInfo() {}
};

/**
 * FDBAChainInfo
 * 连锁信息
 * 用于连锁系统计算
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAChainInfo
{
	GENERATED_BODY()

	/** 当前连锁等级（0-10） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Chain")
	int32 ChainLevel = 0;

	/** 连锁阶段 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Chain")
	EDBAChainTier ChainTier = EDBAChainTier::None;

	/** 伤害加成倍率 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Chain")
	float DamageMultiplier = 1.0f;

	/** 上次命中时间 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Chain")
	float LastHitTime = 0.0f;

	/** 本轮连锁包含的元素类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Chain")
	TSet<EDBAElementType> ElementsInChain;

	FDBAChainInfo() {}
};