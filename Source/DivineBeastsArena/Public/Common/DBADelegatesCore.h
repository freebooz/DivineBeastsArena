// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/DBAResultTypes.h"

/**
 * 核心委托定义
 * 用于跨模块、跨系统的事件通知
 * 所有委托参数必须有详细中文注释
 */

/**
 * 操作完成委托
 * @param Result 操作结果，包含成功标志、错误码、错误消息
 */
DECLARE_DELEGATE_OneParam(FDBAOnOperationComplete, const FDBAOperationResult& /* Result */);

/**
 * 操作完成多播委托
 * @param Result 操作结果
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FDBAOnOperationCompleteMulticast, const FDBAOperationResult& /* Result */);

/**
 * 数据加载完成委托
 * @param Result 数据加载结果，包含资源路径、加载耗时
 */
DECLARE_DELEGATE_OneParam(FDBAOnDataLoadComplete, const FDBADataLoadResult& /* Result */);

/**
 * 数据加载完成多播委托
 * @param Result 数据加载结果
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FDBAOnDataLoadCompleteMulticast, const FDBADataLoadResult& /* Result */);

/**
 * 状态变更委托
 * @param OldState 旧状态名称
 * @param NewState 新状态名称
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FDBAOnStateChanged, FName /* OldState */, FName /* NewState */);

/**
 * 连接状态变更委托
 * @param bConnected 是否已连接
 * @param ErrorCode 错误码，连接成功时为 None
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FDBAOnConnectionStateChanged, bool /* bConnected */, EDBAErrorCode /* ErrorCode */);

/**
 * 玩家加入委托
 * @param PlayerId 玩家唯一标识
 * @param PlayerName 玩家名称
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FDBAOnPlayerJoined, const FString& /* PlayerId */, const FString& /* PlayerName */);

/**
 * 玩家离开委托
 * @param PlayerId 玩家唯一标识
 * @param Reason 离开原因
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FDBAOnPlayerLeft, const FString& /* PlayerId */, const FString& /* Reason */);

/**
 * 匹配状态变更委托
 * @param MatchState 匹配状态名称
 * @param TimeRemaining 剩余时间（秒），-1 表示无限制
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FDBAOnMatchStateChanged, FName /* MatchState */, float /* TimeRemaining */);

/**
 * 技能释放委托
 * @param AbilityId 技能唯一标识
 * @param bSuccess 是否释放成功
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FDBAOnAbilityActivated, int32 /* AbilityId */, bool /* bSuccess */);

/**
 * 伤害事件委托
 * @param SourceActorId 伤害来源 Actor 唯一标识
 * @param TargetActorId 伤害目标 Actor 唯一标识
 * @param Damage 伤害值
 */
DECLARE_MULTICAST_DELEGATE_ThreeParams(FDBAOnDamageDealt, int32 /* SourceActorId */, int32 /* TargetActorId */, float /* Damage */);

/**
 * 击杀事件委托
 * @param KillerActorId 击杀者 Actor 唯一标识
 * @param VictimActorId 被击杀者 Actor 唯一标识
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FDBAOnActorKilled, int32 /* KillerActorId */, int32 /* VictimActorId */);

/**
 * UI 事件委托
 * @param EventName 事件名称
 * @param EventData 事件数据（JSON 格式）
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FDBAOnUIEvent, FName /* EventName */, const FString& /* EventData */);