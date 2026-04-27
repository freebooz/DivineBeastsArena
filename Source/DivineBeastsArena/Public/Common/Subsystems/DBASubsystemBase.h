// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/Subsystem.h"
#include "Common/Subsystems/DBASubsystemImpl.h"
#include "DBASubsystemBase.generated.h"

/**
 * DBA 子系统基类
 *
 * 提供所有 DBA 子系统的通用功能：
 * - 生命周期管理
 * - 依赖检查
 * - 错误恢复
 * - 线程边界标记
 * - 日志与调试支持
 *
 * 所有 DBA 子系统必须继承此类或其派生类
 */
UCLASS(Abstract)
class DIVINEBEASTSARENA_API UDBASubsystemBase : public USubsystem, public DBASubsystemImpl
{
	GENERATED_BODY()

public:
	UDBASubsystemBase();

	/**
	 * 获取子系统显示名称
	 * 用于日志、调试、错误报告
	 */
	virtual FString GetSubsystemDisplayName() const { return GetSubsystemDisplayNameInternal(*GetClass()->GetName().ToString()); }

	/**
	 * 检查子系统是否已正确初始化
	 * 用于依赖检查和错误恢复
	 */
	virtual bool IsSubsystemInitialized() const { return bIsInitialized; }

	/**
	 * 检查子系统是否支持当前运行环境
	 * 例如：Dedicated Server 不支持 UI 相关子系统
	 */
	virtual bool IsSupportedInCurrentEnvironment() const { return true; }

protected:
	/**
	 * 子系统初始化完成标记
	 * 派生类在完成初始化后必须设置为 true
	 */
	UPROPERTY()
	bool bIsInitialized = false;

	/**
	 * 记录子系统错误
	 * 统一错误日志格式，便于调试和监控
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
	 * 用于线程边界检查
	 */
	bool IsInGameThread() const { return IsInGameThreadInternal(); }

	/**
	 * 确保在 GameThread 执行
	 * 如果不在 GameThread 则记录错误
	 */
	bool EnsureGameThread(const FString& FunctionName) const { return EnsureGameThreadInternal(*GetClass()->GetName().ToString(), *FunctionName); }
};
