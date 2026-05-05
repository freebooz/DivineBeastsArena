// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "DBAInputActionSetTypes.generated.h"

/**
 * 核心战斗输入动作集
 * 固定最多 6 个核心战斗输入：BasicAttack + Skill01~04 + Ultimate
 * Passive / Resonance 自动生效，不占输入槽
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBACombatInputActionSet
{
	GENERATED_BODY()

	/** 基础攻击输入动作 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Input", meta = (DisplayName = "基础攻击"))
	TObjectPtr<UInputAction> BasicAttack;

	/** 技能01输入动作 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Input", meta = (DisplayName = "技能01"))
	TObjectPtr<UInputAction> Skill01;

	/** 技能02输入动作 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Input", meta = (DisplayName = "技能02"))
	TObjectPtr<UInputAction> Skill02;

	/** 技能03输入动作 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Input", meta = (DisplayName = "技能03"))
	TObjectPtr<UInputAction> Skill03;

	/** 技能04输入动作 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Input", meta = (DisplayName = "技能04"))
	TObjectPtr<UInputAction> Skill04;

	/** 生肖大招输入动作（ZodiacUltimate） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Input", meta = (DisplayName = "生肖大招"))
	TObjectPtr<UInputAction> Ultimate;
};

/**
 * 系统输入动作集
 * 包含非战斗核心的系统功能输入
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBASystemInputActionSet
{
	GENERATED_BODY()

	/** 锁定目标输入动作 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "System Input", meta = (DisplayName = "锁定目标"))
	TObjectPtr<UInputAction> LockTarget;

	/** Ping 输入动作 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "System Input", meta = (DisplayName = "Ping"))
	TObjectPtr<UInputAction> Ping;

	/** 计分板输入动作 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "System Input", meta = (DisplayName = "计分板"))
	TObjectPtr<UInputAction> Scoreboard;

	/** 菜单输入动作 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "System Input", meta = (DisplayName = "菜单"))
	TObjectPtr<UInputAction> Menu;

	/** 聊天输入动作 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "System Input", meta = (DisplayName = "聊天"))
	TObjectPtr<UInputAction> Chat;

	/** 地图输入动作 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "System Input", meta = (DisplayName = "地图"))
	TObjectPtr<UInputAction> Map;

	/** 交互输入动作 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "System Input", meta = (DisplayName = "交互"))
	TObjectPtr<UInputAction> Interact;
};

/**
 * 移动与摄像机输入动作集
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAMovementInputActionSet
{
	GENERATED_BODY()

	/** 移动输入动作（2D 向量） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement Input", meta = (DisplayName = "移动"))
	TObjectPtr<UInputAction> Move;

	/** 摄像机旋转输入动作（2D 向量） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement Input", meta = (DisplayName = "摄像机旋转"))
	TObjectPtr<UInputAction> Look;

	/** 跳跃输入动作 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement Input", meta = (DisplayName = "跳跃"))
	TObjectPtr<UInputAction> Jump;
};
