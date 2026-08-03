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
#include "GameCore/Core/Subsystems/DBAGameInstanceSubsystemBase.h"
#include "GameCore/Session/Queue/DBAQueueTypes.h"
#include "GameCore/Session/Queue/DBAReadyCheckTypes.h"
#include "GameCore/Session/DBAMatchSessionTypes.h"
#include "DBAQueueServiceBase.generated.h"

/**
 * Queue 服务基类
 *
 * 功能：
 * - 开始匹配
 * - 取消匹配
 * - 匹配更新
 * - 匹配找到
 * - 准备确认
 * - 拒绝准备
 * - 超时处理
 *
 * 实现策略：
 * - 基类提供接口定义和通用逻辑
 * - 派生类实现具体的 Queue 逻辑
 * - OnlineQueueService：在线模式，连接外部 Matchmaking 服务（未来实现）
 *
 * 外部服务边界：
 * - 外部 Matchmaking 服务不可用时返回明确失败结果，不伪造匹配结果
 * - 所有外部请求必须异步、可超时、可丢弃、可熔断、可禁用
 * - 客户端不能决定最终 Match / Travel 结果
 *
 * Dedicated Server：
 * - Dedicated Server 不调用 Queue 接口
 * - Dedicated Server 只验证客户端提供的 Match 信息
 */
UCLASS()
class GAMECORE_API UDBAQueueServiceBase : public UDBAGameInstanceSubsystemBase
{
	GENERATED_BODY()

public:
	UDBAQueueServiceBase();

	// UDBAGameInstanceSubsystemBase interface
	virtual void OnSubsystemInitialize() override;
	virtual void OnSubsystemDeinitialize() override;
	virtual bool IsSupportedInCurrentEnvironment() const override;
	// End of UDBAGameInstanceSubsystemBase interface

	/**
	 * 开始匹配
	 *
	 * @param QueueType 队列类型
	 * @param OnComplete 完成回调
	 */
	virtual void StartQueue(EDBAQueueType QueueType, FDBAOnQueueStarted OnComplete);

	/**
	 * 取消匹配
	 *
	 * @param OnComplete 完成回调
	 */
	virtual void CancelQueue(FDBAOnQueueCancelled OnComplete);

	/**
	 * 确认准备
	 *
	 * @param ReadyCheckId 准备确认 ID
	 * @param OnComplete 完成回调
	 */
	virtual void ConfirmReady(const FDBAReadyCheckId& ReadyCheckId, FDBAOnReadyCheckCompleted OnComplete);

	/**
	 * 拒绝准备
	 *
	 * @param ReadyCheckId 准备确认 ID
	 * @param OnComplete 完成回调
	 */
	virtual void DeclineReady(const FDBAReadyCheckId& ReadyCheckId, FDBAOnReadyCheckCompleted OnComplete);

	/**
	 * 获取当前 Queue 信息
	 */
	UFUNCTION(BlueprintCallable, Category = "Queue")
	const FDBAQueueInfo& GetCurrentQueueInfo() const { return CurrentQueueInfo; }

	/**
	 * 获取当前 ReadyCheck 信息
	 */
	UFUNCTION(BlueprintCallable, Category = "Queue")
	const FDBAReadyCheckInfo& GetCurrentReadyCheckInfo() const { return CurrentReadyCheckInfo; }

	/**
	 * 检查是否在队列中
	 */
	UFUNCTION(BlueprintCallable, Category = "Queue")
	bool IsInQueue() const { return CurrentQueueInfo.IsValid(); }

	/**
	 * 检查是否在准备确认中
	 */
	UFUNCTION(BlueprintCallable, Category = "Queue")
	bool IsInReadyCheck() const { return CurrentReadyCheckInfo.IsValid(); }

	/**
	 * Queue 更新委托
	 */
	FDBAOnQueueUpdated OnQueueUpdated;

	/**
	 * 匹配找到委托
	 */
	FDBAOnMatchFound OnMatchFound;

	/**
	 * ReadyCheck 开始委托
	 */
	FDBAOnReadyCheckStarted OnReadyCheckStarted;

	/**
	 * ReadyCheck 更新委托
	 */
	FDBAOnReadyCheckUpdated OnReadyCheckUpdated;

	/**
	 * ReadyCheck 完成委托
	 */
	FDBAOnReadyCheckCompleted OnReadyCheckCompleted;

	/**
	 * ReadyCheck 取消委托
	 */
	FDBAOnReadyCheckCancelled OnReadyCheckCancelled;

	/**
	 * ReadyCheck 超时委托
	 */
	FDBAOnReadyCheckTimeout OnReadyCheckTimeout;

protected:
	/**
	 * 当前 Queue 信息
	 */
	UPROPERTY()
	FDBAQueueInfo CurrentQueueInfo;

	/**
	 * 当前 ReadyCheck 信息
	 */
	UPROPERTY()
	FDBAReadyCheckInfo CurrentReadyCheckInfo;

	/**
	 * 生成 Queue ID
	 */
	FDBAQueueId GenerateQueueId();

	/**
	 * 生成 ReadyCheck ID
	 */
	FDBAReadyCheckId GenerateReadyCheckId();

	/**
	 * 生成 MatchSession ID
	 */
	FDBAMatchSessionId GenerateMatchSessionId();
};
