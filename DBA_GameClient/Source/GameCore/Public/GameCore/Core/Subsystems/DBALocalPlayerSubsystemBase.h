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
#include "Subsystems/LocalPlayerSubsystem.h"
#include "GameCore/Core/Subsystems/DBASubsystemImpl.h"
#include "DBALocalPlayerSubsystemBase.generated.h"

/**
 * DBA LocalPlayer 子系统基类
 *
 * 生命周期：
 * - 随 LocalPlayer 创建而创建
 * - 随 LocalPlayer 销毁而销毁
 * - 每个本地玩家都有独立的实例
 *
 * 适用场景：
 * - 玩家输入系统
 * - 玩家 UI 系统
 * - 玩家设置
 * - 玩家本地缓存
 * - 玩家音频设置
 *
 * 不适用场景：
 * - 对局内系统（使用 World Subsystem）
 * - 跨关卡持久化数据（使用 GameInstance Subsystem）
 *
 * 线程安全：
 * - Initialize / Deinitialize 在 GameThread
 * - 异步回调必须使用 AsyncTask(ENamedThreads::GameThread, ...) 切回 GameThread
 *
 * Dedicated Server：
 * - LocalPlayer Subsystem 只在 Client 创建
 * - Dedicated Server 没有 LocalPlayer，不会创建此类子系统
 *
 * 分屏支持：
 * - 每个本地玩家都有独立的 LocalPlayer Subsystem 实例
 * - 适合处理玩家特定的输入、UI、设置
 */
UCLASS(Abstract)
class GAMECORE_API UDBALocalPlayerSubsystemBase : public ULocalPlayerSubsystem, public DBASubsystemImpl
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
	virtual FString GetSubsystemDisplayName() const { return GetSubsystemDisplayNameInternal(GetClass()->GetName()); }

	/**
	 * 检查子系统是否已正确初始化
	 */
	virtual bool IsSubsystemInitialized() const { return bIsInitialized; }

	/**
	 * 检查子系统是否支持当前运行环境
	 * LocalPlayer Subsystem 只在 Client 支持
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
	 * 在玩家退出时调用
	 */
	virtual void CancelAllAsyncOperations() {}

	/**
	 * 记录子系统错误
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
	 */
	bool IsInGameThread() const { return IsInGameThreadInternal(); }

	/**
	 * 确保在 GameThread 执行
	 */
	bool EnsureGameThread(const FString& FunctionName) const { return EnsureGameThreadInternal(GetClass()->GetName(), FunctionName); }
};
