// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/Subsystems/DBAGameInstanceSubsystemBase.h"
#include "Common/Queue/DBAQueueTypes.h"
#include "Common/Queue/DBAReadyCheckTypes.h"
#include "Common/Session/DBAMatchSessionTypes.h"
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
 * - MockQueueService：本地模拟，用于开发和离线测试
 * - OnlineQueueService：在线模式，连接外部 Matchmaking 服务（未来实现）
 *
 * 外部服务边界：
 * - 外部 Matchmaking 服务不可用时，自动降级到 MockQueueService
 * - 所有外部请求必须异步、可超时、可丢弃、可熔断、可禁用
 * - 客户端不能决定最终 Match / Travel 结果
 *
 * Dedicated Server：
 * - Dedicated Server 不调用 Queue 接口
 * - Dedicated Server 只验证客户端提供的 Match 信息
 */
UCLASS(Abstract)
class DIVINEBEASTSARENA_API UDBAQueueServiceBase : public UDBAGameInstanceSubsystemBase
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
