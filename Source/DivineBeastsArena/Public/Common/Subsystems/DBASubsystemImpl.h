// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/DBALogChannels.h"

/**
 * DBA 子系统实现混入类
 *
 * 提供所有 DBA 子系统共用的实现代码：
 * - 日志方法（LogSubsystemError/Warning/Info）
 * - 线程检查方法（IsInGameThread/EnsureGameThread）
 *
 * 使用方式：让子系统基类多继承此类
 * class UMySubsystem : public USubsystem, public DBASubsystemImpl { ... }
 *
 * 这样可以避免代码重复，同时保持各自的 UE 子系统继承链完整
 */
class DIVINEBEASTSARENA_API DBASubsystemImpl
{
protected:
	/**
	 * 获取子系统显示名称
	 */
	FString GetSubsystemDisplayNameInternal(const TCHAR* ClassName) const
	{
		return ClassName;
	}

	/**
	 * 记录子系统错误
	 */
	void LogSubsystemErrorInternal(const TCHAR* SubsystemName, const FString& ErrorMessage) const
	{
		UE_LOG(LogDBACore, Error, TEXT("[%s] %s"), SubsystemName, *ErrorMessage);
	}

	/**
	 * 记录子系统警告
	 */
	void LogSubsystemWarningInternal(const TCHAR* SubsystemName, const FString& WarningMessage) const
	{
		UE_LOG(LogDBACore, Warning, TEXT("[%s] %s"), SubsystemName, *WarningMessage);
	}

	/**
	 * 记录子系统信息
	 */
	void LogSubsystemInfoInternal(const TCHAR* SubsystemName, const FString& InfoMessage) const
	{
		UE_LOG(LogDBACore, Log, TEXT("[%s] %s"), SubsystemName, *InfoMessage);
	}

	/**
	 * 检查是否在 GameThread
	 */
	bool IsInGameThreadInternal() const
	{
		return ::IsInGameThread();
	}

	/**
	 * 确保在 GameThread 执行
	 */
	bool EnsureGameThreadInternal(const TCHAR* SubsystemName, const TCHAR* FunctionName) const
	{
		if (!::IsInGameThread())
		{
			UE_LOG(LogDBACore, Error, TEXT("[%s] %s called from non-game thread"), SubsystemName, FunctionName);
			return false;
		}
		return true;
	}
};
