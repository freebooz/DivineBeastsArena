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
#include "GameCore/Core/DBALogChannels.h"

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
class GAMECORE_API DBASubsystemImpl
{
protected:
	/**
	 * 获取子系统显示名称
	 */
	FString GetSubsystemDisplayNameInternal(const FString& ClassName) const
	{
		return ClassName;
	}

	/**
	 * 记录子系统错误
	 */
	void LogSubsystemErrorInternal(const FString& SubsystemName, const FString& ErrorMessage) const
	{
		UE_LOG(LogDBACore, Error, TEXT("[%s] %s"), *SubsystemName, *ErrorMessage);
	}

	/**
	 * 记录子系统警告
	 */
	void LogSubsystemWarningInternal(const FString& SubsystemName, const FString& WarningMessage) const
	{
		UE_LOG(LogDBACore, Warning, TEXT("[%s] %s"), *SubsystemName, *WarningMessage);
	}

	/**
	 * 记录子系统信息
	 */
	void LogSubsystemInfoInternal(const FString& SubsystemName, const FString& InfoMessage) const
	{
		UE_LOG(LogDBACore, Log, TEXT("[%s] %s"), *SubsystemName, *InfoMessage);
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
	bool EnsureGameThreadInternal(const FString& SubsystemName, const FString& FunctionName) const
	{
		if (!::IsInGameThread())
		{
			UE_LOG(LogDBACore, Error, TEXT("[%s] %s 在非游戏线程调用"), *SubsystemName, *FunctionName);
			return false;
		}
		return true;
	}

public:
	/**
	 * 便捷日志方法：记录信息日志
	 * @param Message 日志信息
	 */
	void LogSubsystemInfo(const TCHAR* Message) const
	{
		UE_LOG(LogDBACore, Log, TEXT("%s"), Message);
	}

	/**
	 * 便捷日志方法：记录信息日志（FString 版本）
	 * @param Message 日志信息
	 */
	void LogSubsystemInfo(const FString& Message) const
	{
		UE_LOG(LogDBACore, Log, TEXT("%s"), *Message);
	}

	/**
	 * 便捷日志方法：记录警告日志
	 * @param Message 警告信息
	 */
	void LogSubsystemWarning(const TCHAR* Message) const
	{
		UE_LOG(LogDBACore, Warning, TEXT("%s"), Message);
	}

	/**
	 * 便捷日志方法：记录警告日志（FString 版本）
	 * @param Message 警告信息
	 */
	void LogSubsystemWarning(const FString& Message) const
	{
		UE_LOG(LogDBACore, Warning, TEXT("%s"), *Message);
	}

	/**
	 * 便捷日志方法：记录错误日志
	 * @param Message 错误信息
	 */
	void LogSubsystemError(const TCHAR* Message) const
	{
		UE_LOG(LogDBACore, Error, TEXT("%s"), Message);
	}

	/**
	 * 便捷日志方法：记录错误日志（FString 版本）
	 * @param Message 错误信息
	 */
	void LogSubsystemError(const FString& Message) const
	{
		UE_LOG(LogDBACore, Error, TEXT("%s"), *Message);
	}
};
