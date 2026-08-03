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
#include "Subsystems/Subsystem.h"
#include "GameCore/Core/Subsystems/DBASubsystemImpl.h"
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
class GAMECORE_API UDBASubsystemBase : public USubsystem, public DBASubsystemImpl
{
	GENERATED_BODY()

public:
	UDBASubsystemBase();

	/**
	 * 获取子系统显示名称
	 * 用于日志、调试、错误报告
	 */
	virtual FString GetSubsystemDisplayName() const { return GetSubsystemDisplayNameInternal(GetClass()->GetName()); }

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
	void LogSubsystemError(const FString& ErrorMessage) const { LogSubsystemErrorInternal(GetClass()->GetName(), ErrorMessage); }

	/**
	 * 记录子系统警告
	 */
	void LogSubsystemWarning(const FString& WarningMessage) const { LogSubsystemWarningInternal(GetClass()->GetName(), WarningMessage); }

	/**
	 * 记录子系统信息
	 */
	void LogSubsystemInfo(const FString& InfoMessage) const { LogSubsystemInfoInternal(GetClass()->GetName(), InfoMessage); }

	/**
	 * 检查是否在 GameThread
	 * 用于线程边界检查
	 */
	bool IsInGameThread() const { return IsInGameThreadInternal(); }

	/**
	 * 确保在 GameThread 执行
	 * 如果不在 GameThread 则记录错误
	 */
	bool EnsureGameThread(const FString& FunctionName) const { return EnsureGameThreadInternal(GetClass()->GetName(), FunctionName); }
};
