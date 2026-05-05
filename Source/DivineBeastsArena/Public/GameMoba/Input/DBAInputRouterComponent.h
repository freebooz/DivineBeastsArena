// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnhancedInputComponent.h"
#include "DBAInputRouterComponent.generated.h"

class UDBAInputConfigDataAsset;
class UDBAInputPlatformPolicy;
class UInputMappingContext;
class UEnhancedInputLocalPlayerSubsystem;

// ========================================
// UI 事件广播（供 UI 层订阅）
// ========================================

/**
 * Ping 位置广播事件
 */
DECLARE_EVENT_OneParam(UDBAInputRouterComponent, FDBAOnPingTriggered, FVector /* PingLocation */)

/**
 * 计分板切换事件
 */
DECLARE_EVENT(UDBAInputRouterComponent, FDBAOnScoreboardTriggered)

/**
 * 菜单切换事件
 */
DECLARE_EVENT(UDBAInputRouterComponent, FDBAOnMenuTriggered)

/**
 * 聊天触发事件
 */
DECLARE_EVENT(UDBAInputRouterComponent, FDBAOnChatTriggered)

/**
 * 地图切换事件
 */
DECLARE_EVENT(UDBAInputRouterComponent, FDBAOnMapTriggered)

/**
 * 锁定目标切换事件
 */
DECLARE_EVENT_OneParam(UDBAInputRouterComponent, FDBAOnLockTargetChanged, AActor* /* NewTarget */)

/**
 * 输入路由组件
 * 负责：
 * - 根据平台加载对应的输入映射上下文
 * - 绑定输入动作到 GAS / Targeting 系统
 * - 处理客户端本地反馈与服务端裁定边界
 * - 提供输入调试日志开关（VS Code 调试时使用）
 *
 * 挂载到 PlayerController 或 Pawn
 * Dedicated Server 不创建此组件
 */
UCLASS(ClassGroup = (Input), meta = (BlueprintSpawnableComponent, DisplayName = "输入路由组件"))
class DIVINEBEASTSARENA_API UDBAInputRouterComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDBAInputRouterComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	/**
	 * 初始化输入路由
	 * @param InputConfig 输入配置数据资产
	 * @param PlatformPolicy 平台策略对象
	 */
	UFUNCTION(BlueprintCallable, Category = "Input Router")
	void InitializeInputRouter(UDBAInputConfigDataAsset* InputConfig, UDBAInputPlatformPolicy* PlatformPolicy);

	/**
	 * 启用输入调试日志
	 * 用于 VS Code 调试输入绑定时查看详细日志
	 */
	UFUNCTION(BlueprintCallable, Category = "Input Router|Debug")
	void EnableInputDebugLog(bool bEnable);

private:
	/** 输入配置数据资产 */
	UPROPERTY(Transient)
	TObjectPtr<UDBAInputConfigDataAsset> InputConfigAsset;

	/** 平台策略对象 */
	UPROPERTY(Transient)
	TObjectPtr<UDBAInputPlatformPolicy> PlatformPolicyObject;

	/** 是否启用输入调试日志 */
	bool bInputDebugLogEnabled = false;

	/** ========== RPC 频率限制 ========== */

	/** Ping RPC 最小间隔时间（秒） */
	static constexpr float MinPingInterval = 0.5f;

	/** LockTarget RPC 最小间隔时间（秒） */
	static constexpr float MinLockTargetInterval = 0.3f;

	/** Interact RPC 最小间隔时间（秒） */
	static constexpr float MinInteractInterval = 0.2f;

	/** 上次 Ping RPC 时间戳 */
	float LastPingTime = 0.0f;

	/** 上次 LockTarget RPC 时间戳 */
	float LastLockTargetTime = 0.0f;

	/** 上次 Interact RPC 时间戳 */
	float LastInteractTime = 0.0f;

	/** Ping 事件广播 */
	FDBAOnPingTriggered OnPingTriggeredEvent;

	/** 计分板切换事件广播 */
	FDBAOnScoreboardTriggered OnScoreboardTriggeredEvent;

	/** 菜单切换事件广播 */
	FDBAOnMenuTriggered OnMenuTriggeredEvent;

	/** 聊天触发事件广播 */
	FDBAOnChatTriggered OnChatTriggeredEvent;

	/** 地图切换事件广播 */
	FDBAOnMapTriggered OnMapTriggeredEvent;

	/** 锁定目标切换事件广播 */
	FDBAOnLockTargetChanged OnLockTargetChangedEvent;

	/** 加载并应用输入映射上下文 */
	void LoadAndApplyMappingContexts();

	/** 绑定输入动作 */
	void BindInputActions();

	/** 获取 EnhancedInput 子系统 */
	UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubsystem() const;

	/** 检查是否可以发送 Ping RPC（频率限制） */
	bool CanSendPingRPC();

	/** 检查是否可以发送 LockTarget RPC（频率限制） */
	bool CanSendLockTargetRPC();

	/** 检查是否可以发送 Interact RPC（频率限制） */
	bool CanSendInteractRPC();

	/** 更新 Ping RPC 时间戳 */
	void UpdatePingTimestamp();

	/** 更新 LockTarget RPC 时间戳 */
	void UpdateLockTargetTimestamp();

	/** 更新 Interact RPC 时间戳 */
	void UpdateInteractTimestamp();

	/** 检查与目标之间是否有视线（用于锁定目标验证） */
	bool HasLineOfSightToTarget(APawn* SourcePawn, AActor* TargetActor) const;

	// ========== 输入回调函数 ==========

	/** 基础攻击输入回调 */
	void OnBasicAttackTriggered(const FInputActionValue& Value);

	/** 技能01输入回调 */
	void OnSkill01Triggered(const FInputActionValue& Value);

	/** 技能02输入回调 */
	void OnSkill02Triggered(const FInputActionValue& Value);

	/** 技能03输入回调 */
	void OnSkill03Triggered(const FInputActionValue& Value);

	/** 技能04输入回调 */
	void OnSkill04Triggered(const FInputActionValue& Value);

	/** 生肖大招输入回调 */
	void OnUltimateTriggered(const FInputActionValue& Value);

	/** 锁定目标输入回调 */
	void OnLockTargetTriggered(const FInputActionValue& Value);

	/** Ping 输入回调 */
	void OnPingTriggered(const FInputActionValue& Value);

	/** 计分板输入回调 */
	void OnScoreboardTriggered(const FInputActionValue& Value);

	/** 菜单输入回调 */
	void OnMenuTriggered(const FInputActionValue& Value);

	// ========================================
// 网络安全 RPC (WithValidation)
// ========================================

/**
 * 发送 Ping 到服务端（带验证）
 * 客户端 -> 服务端 RPC，带验证防止作弊
 */
UFUNCTION(Server, Reliable, WithValidation, Category = "Input Router|Network")
void ServerPing(FVector_NetQuantize10 Location);

/**
 * 锁定目标切换请求（带验证）
 * 客户端 -> 服务端 RPC，带验证防止非法目标
 */
UFUNCTION(Server, Reliable, WithValidation, Category = "Input Router|Network")
void ServerLockTarget(AActor* TargetActor);

/**
 * 交互请求（带验证）
 * 客户端 -> 服务端 RPC，带验证防止非法交互
 */
UFUNCTION(Server, Reliable, WithValidation, Category = "Input Router|Network")
void ServerInteract(AActor* InteractableActor);

	/** 聊天输入回调 */
	void OnChatTriggered(const FInputActionValue& Value);

	/** 地图输入回调 */
	void OnMapTriggered(const FInputActionValue& Value);

	/** 交互输入回调 */
	void OnInteractTriggered(const FInputActionValue& Value);

	/** 移动输入回调 */
	void OnMoveTriggered(const FInputActionValue& Value);

	/** 摄像机旋转输入回调 */
	void OnLookTriggered(const FInputActionValue& Value);

	/** 跳跃输入回调 */
	void OnJumpTriggered(const FInputActionValue& Value);

	/** 输出调试日志 */
	void LogInputDebug(const FString& Message) const;
};
