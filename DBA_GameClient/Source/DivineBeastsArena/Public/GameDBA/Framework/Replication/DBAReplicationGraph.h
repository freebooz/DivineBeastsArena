// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明十二生肖竞技场专用复制图类型。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

#pragma once

#include "CoreMinimal.h"
#include "ReplicationGraph.h"
#include "DBAReplicationGraph.generated.h"

struct FNewReplicatedActorInfo;

/**
 * 十二生肖竞技场专用复制图。
 * 使用空间分区网格管理角色、投射物和小兵的复制相关性，
 * 同时将 PlayerState 和 RPC 处理器归入全局相关类别。
 */
UCLASS()
class DIVINEBEASTSARENA_API UDBAReplicationGraph : public UReplicationGraph
{
	GENERATED_BODY()

public:
	virtual void InitGlobalActorClassSettings() override;
	virtual void InitGlobalGraphNodes() override;
	virtual void RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalInfo) override;
	virtual void RouteRemoveNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo) override;

private:
	/** 空间分区网格节点 */
	UPROPERTY()
	TObjectPtr<UReplicationGraphNode_GridSpatialization2D> GridNode;

	/** 全局相关 Actor 节点（PlayerState、GameMode 等） */
	UPROPERTY()
	TObjectPtr<UReplicationGraphNode_ActorList> AlwaysRelevantNode;

	/** 静态空间节点（不移动的 Actor） */
	UPROPERTY()
	TObjectPtr<UReplicationGraphNode_ActorList> StaticSpatialNode;
};
