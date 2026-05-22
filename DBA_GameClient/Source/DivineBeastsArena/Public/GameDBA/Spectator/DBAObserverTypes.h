// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#pragma once

#include "CoreMinimal.h"
#include "DBAObserverTypes.generated.h"

class ADBAZodiacCharacterBase;

/**
 * EDBAObserverControlLevel
 * 观战控制权限等级
 */
UENUM(BlueprintType)
enum class EDBAObserverControlLevel : uint8
{
	None        UMETA(DisplayName = "无权限"),
	ViewOnly    UMETA(DisplayName = "仅观看"),
	Pause       UMETA(DisplayName = "可暂停"),
	Kick        UMETA(DisplayName = "可踢人"),
	Admin       UMETA(DisplayName = "完全控制")
};

/**
 * EDBAObserverViewMode
 * 观战视角模式
 */
UENUM(BlueprintType)
enum class EDBAObserverViewMode : uint8
{
	Follow      UMETA(DisplayName = "跟随视角"),
	Free        UMETA(DisplayName = "自由视角"),
	Tactical    UMETA(DisplayName = "战术视角")
};

/**
 * FDBAObserverViewTarget
 * 观战视角目标数据
 * 同步当前观看的玩家信息
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAObserverViewTarget
{
	GENERATED_BODY()

	FDBAObserverViewTarget()
		: TargetCharacter(nullptr)
		, PlayerName(NAME_None)
		, TeamID(0)
		, HeroID(NAME_None)
		, CurrentHP(0.0f)
		, MaxHP(0.0f)
		, CurrentEnergy(0.0f)
		, MaxEnergy(0.0f)
		, bUltimateReady(false)
		, UltimateEnergy(0.0f)
	{}

	/** 观看的玩家Actor */
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<ADBAZodiacCharacterBase> TargetCharacter;

	/** 玩家名称 */
	UPROPERTY(BlueprintReadOnly)
	FName PlayerName;

	/** 队伍ID */
	UPROPERTY(BlueprintReadOnly)
	uint8 TeamID;

	/** 英雄类型ID */
	UPROPERTY(BlueprintReadOnly)
	FName HeroID;

	/** 当前HP */
	UPROPERTY(BlueprintReadOnly)
	float CurrentHP;

	/** 最大HP */
	UPROPERTY(BlueprintReadOnly)
	float MaxHP;

	/** 当前能量 */
	UPROPERTY(BlueprintReadOnly)
	float CurrentEnergy;

	/** 最大能量 */
	UPROPERTY(BlueprintReadOnly)
	float MaxEnergy;

	/** 终极技能是否就绪 */
	UPROPERTY(BlueprintReadOnly)
	bool bUltimateReady;

	/** 终极技能能量 */
	UPROPERTY(BlueprintReadOnly)
	float UltimateEnergy;

	/** 技能冷却数组 (Q, W, E, R) */
	UPROPERTY(BlueprintReadOnly)
	TArray<float> SkillCooldowns;

	/** 技能最大冷却数组 */
	UPROPERTY(BlueprintReadOnly)
	TArray<float> SkillMaxCooldowns;

	/** 获取HP百分比 */
	float GetHPPercent() const { return MaxHP > 0.0f ? CurrentHP / MaxHP : 0.0f; }

	/** 获取能量百分比 */
	float GetEnergyPercent() const { return MaxEnergy > 0.0f ? CurrentEnergy / MaxEnergy : 0.0f; }
};

/**
 * FDBAObserverInfo
 * 观战者信息
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAObserverInfo
{
	GENERATED_BODY()

	FDBAObserverInfo()
		: ObserverID(FUniqueNetIdRepl())
		, ObserverName(NAME_None)
		, ControlLevel(EDBAObserverControlLevel::ViewOnly)
		, CurrentViewTargetIndex(INDEX_NONE)
		, ViewMode(EDBAObserverViewMode::Follow)
	{}

	/** 观战者唯一ID */
	UPROPERTY(BlueprintReadOnly)
	FUniqueNetIdRepl ObserverID;

	/** 观战者名称 */
	UPROPERTY(BlueprintReadOnly)
	FName ObserverName;

	/** 控制权限等级 */
	UPROPERTY(BlueprintReadOnly)
	EDBAObserverControlLevel ControlLevel;

	/** 当前视角目标索引 */
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentViewTargetIndex;

	/** 视角模式 */
	UPROPERTY(BlueprintReadOnly)
	EDBAObserverViewMode ViewMode;

	/** 是否已连接 */
	bool IsConnected() const { return ObserverID.IsValid(); }
};

/**
 * FDBAObserverCameraState
 * 观战相机状态
 */
USTRUCT()
struct DIVINEBEASTSARENA_API FDBAObserverCameraState
{
	GENERATED_BODY()

	FDBAObserverCameraState()
		: CameraLocation(FVector::ZeroVector)
		, CameraRotation(FRotator::ZeroRotator)
		, CameraZoom(1000.0f)
	{}

	/** 相机位置 */
	UPROPERTY(Transient)
	FVector CameraLocation;

	/** 相机旋转 */
	UPROPERTY(Transient)
	FRotator CameraRotation;

	/** 缩放距离 */
	UPROPERTY(Transient)
	float CameraZoom;
};
