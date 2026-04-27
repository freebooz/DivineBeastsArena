// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Common/Subsystems/DBASubsystemImpl.h"
#include "DBAGameInstanceSubsystemBase.generated.h"

/**
 * DBA GameInstance 子系统基类
 *
 * 生命周期：
 * - 随 GameInstance 创建而创建
 * - 随 GameInstance 销毁而销毁
 * - 跨关卡持久化，不会因为关卡切换而销毁
 *
 * 适用场景：
 * - 账户系统
 * - 好友系统
 * - 组队系统
 * - 匹配队列
 * - 全局配置
 * - 跨关卡持久化数据
 * - 外部服务客户端（Monitoring / GameOps）
 *
 * 不适用场景：
 * - 对局内系统（使用 World Subsystem）
 * - 玩家本地数据（使用 LocalPlayer Subsystem）
 *
 * 线程安全：
 * - Initialize / Deinitialize 在 GameThread
 * - 异步回调必须使用 AsyncTask(ENamedThreads::GameThread, ...) 切回 GameThread
 *
 * Dedicated Server：
 * - GameInstance Subsystem 在 Server 和 Client 都会创建
 * - 派生类必须通过 IsSupportedInCurrentEnvironment() 判断是否支持当前环境
 *
 * 外部服务集成：
 * - 外部服务（Monitoring / GameOps）客户端应作为 GameInstance Subsystem
 * - 外部服务不可用时游戏必须正常运行
 * - 所有外部请求必须异步、可超时、可丢弃、可熔断、可禁用
 * - 外部服务回调必须检查 UObject 生命周期
 */
UCLASS(Abstract)
class DIVINEBEASTSARENA_API UDBAGameInstanceSubsystemBase : public UGameInstanceSubsystem, public DBASubsystemImpl
{
	GENERATED_BODY()

public:
	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	// End of USubsystem interface

	/**
	 * 获取子系统显示名称
	 */
	virtual FString GetSubsystemDisplayName() const { return GetSubsystemDisplayNameInternal(*FName(GetClass()->GetName()).ToString()); }

	/**
	 * 检查子系统是否已正确初始化
	 */
	virtual bool IsSubsystemInitialized() const { return bIsInitialized; }

	/**
	 * 检查子系统是否支持当前运行环境
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
	 */
	virtual void OnSubsystemInitialize() {}

	/**
	 * 派生类反初始化逻辑
	 */
	virtual void OnSubsystemDeinitialize() {}

	/**
	 * 取消所有异步操作
	 * 在游戏退出时调用
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
