// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Common/Subsystems/DBASubsystemImpl.h"
#include "DBAWorldSubsystemBase.generated.h"

/**
 * DBA World 子系统基类
 *
 * 生命周期：
 * - 随 UWorld 创建而创建
 * - 随 UWorld 销毁而销毁
 * - 关卡切换时会销毁旧 World 的子系统，创建新 World 的子系统
 *
 * 适用场景：
 * - 对局内系统（战斗、AI、技能、Buff）
 * - 关卡相关系统（地形、传送门、目标点）
 * - 需要随关卡切换重置的系统
 *
 * 不适用场景：
 * - 跨关卡持久化数据（使用 GameInstance Subsystem）
 * - 玩家本地数据（使用 LocalPlayer Subsystem）
 *
 * 线程安全：
 * - Initialize / Deinitialize 在 GameThread
 * - Tick 在 GameThread
 * - 异步回调必须使用 AsyncTask(ENamedThreads::GameThread, ...) 切回 GameThread
 *
 * Dedicated Server：
 * - World Subsystem 在 Server 和 Client 都会创建
 * - 派生类必须通过 IsSupportedInCurrentEnvironment() 判断是否支持当前环境
 */
UCLASS(Abstract)
class DIVINEBEASTSARENA_API UDBAWorldSubsystemBase : public UWorldSubsystem, public DBASubsystemImpl
{
	GENERATED_BODY()

public:
	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	// End of USubsystem interface

	// UWorldSubsystem interface
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	// End of UWorldSubsystem interface

	/**
	 * 获取子系统显示名称
	 */
	virtual FString GetSubsystemDisplayName() const { return GetSubsystemDisplayNameInternal(*GetClass()->GetName().ToString()); }

	/**
	 * 检查子系统是否已正确初始化
	 */
	virtual bool IsSubsystemInitialized() const { return bIsInitialized; }

	/**
	 * 检查子系统是否支持当前运行环境
	 * 派生类可重写此方法，例如 UI 子系统在 Dedicated Server 返回 false
	 */
	virtual bool IsSupportedInCurrentEnvironment() const;

protected:
	/**
	 * 子系统初始化完成标记
	 */
	UPROPERTY()
	bool bIsInitialized = false;

	/**
	 * 派生类初始化逻辑
	 * 在 Initialize() 中调用，此时依赖的其他子系统可能尚未初始化
	 */
	virtual void OnSubsystemInitialize() {}

	/**
	 * 派生类反初始化逻辑
	 * 在 Deinitialize() 中调用
	 */
	virtual void OnSubsystemDeinitialize() {}

	/**
	 * World BeginPlay 回调
	 * 此时所有子系统已初始化，可以安全访问依赖
	 */
	virtual void OnWorldBeginPlayInternal() {}

	/**
	 * 取消所有异步操作
	 * 在关卡切换、断线、返回大厅时调用
	 * 派生类必须实现此方法，取消所有未完成的异步操作
	 */
	virtual void CancelAllAsyncOperations() {}

	/**
	 * 记录子系统错误
	 */
	void LogSubsystemError(const FString& ErrorMessage) const { LogSubsystemErrorInternal(*GetClass()->GetName().ToString(), ErrorMessage); }

	/**
	 * 记录子系统警告
	 */
	void LogSubsystemWarning(const FString& WarningMessage) const { LogSubsystemWarningInternal(*GetClass()->GetName().ToString(), WarningMessage); }

	/**
	 * 记录子系统信息
	 */
	void LogSubsystemInfo(const FString& InfoMessage) const { LogSubsystemInfoInternal(*GetClass()->GetName().ToString(), InfoMessage); }

	/**
	 * 检查是否在 GameThread
	 */
	bool IsInGameThread() const { return IsInGameThreadInternal(); }

	/**
	 * 确保在 GameThread 执行
	 */
	bool EnsureGameThread(const FString& FunctionName) const { return EnsureGameThreadInternal(*GetClass()->GetName().ToString(), *FunctionName); }
};
