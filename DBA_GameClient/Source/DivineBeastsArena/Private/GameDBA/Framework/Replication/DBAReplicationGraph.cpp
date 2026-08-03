// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载十二生肖竞技场复制图的初始化与路由逻辑。
- 阅读重点：先看 InitGlobalActorClassSettings 与 InitGlobalGraphNodes 的节点布局，再看 RouteAdd/Remove 的路由策略。
- 修改提示：保持现有分层边界；新增节点或类配置时，应延续空间分区+全局相关的二分策略。
*/

#include "GameDBA/Framework/Replication/DBAReplicationGraph.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/Gameplay/Abilities/Projectiles/DBASkillProjectileBase.h"
#include "ReplicationGraph.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "UObject/UObjectIterator.h"

void UDBAReplicationGraph::InitGlobalActorClassSettings()
{
	Super::InitGlobalActorClassSettings();

	// 遍历所有 Actor 类，按 DBA 业务需求设置复制参数
	for (TObjectIterator<UClass> It; It; ++It)
	{
		UClass* Class = *It;
		AActor* ActorCDO = Cast<AActor>(Class->GetDefaultObject());
		if (!ActorCDO || !ActorCDO->GetIsReplicated())
		{
			continue;
		}

		// 跳过 SKEL 和 REINST 临时类
		if (Class->GetName().StartsWith(TEXT("SKEL_")) || Class->GetName().StartsWith(TEXT("REINST_")))
		{
			continue;
		}

		FClassReplicationInfo ClassInfo;

		// 根据类类型确定目标更新频率：角色 30Hz、投射物 20Hz、PlayerState 10Hz、其他取 CDO 默认值
		float DesiredFrequency = ActorCDO->GetNetUpdateFrequency();
		if (Class->IsChildOf(ACharacter::StaticClass()) || Class->IsChildOf(APawn::StaticClass()))
		{
			DesiredFrequency = 30.0f;
		}
		else if (Class->IsChildOf(ADBASkillProjectileBase::StaticClass()))
		{
			DesiredFrequency = 20.0f;
		}
		else if (Class->IsChildOf(APlayerState::StaticClass()))
		{
			DesiredFrequency = 10.0f;
		}

		// 复制图基于帧，将频率转换为复制周期帧数
		ClassInfo.ReplicationPeriodFrame = GetReplicationPeriodFrameForFrequency(DesiredFrequency);

		// 全局相关 Actor（bAlwaysRelevant/bOnlyRelevantToOwner）取消距离剔除
		if (ActorCDO->bAlwaysRelevant || ActorCDO->bOnlyRelevantToOwner)
		{
			ClassInfo.SetCullDistanceSquared(0.0f);
		}
		else
		{
			ClassInfo.SetCullDistanceSquared(ActorCDO->GetNetCullDistanceSquared());
		}

		GlobalActorReplicationInfoMap.SetClassInfo(Class, ClassInfo);
	}

	UE_LOG(LogDBANetwork, Log, TEXT("[复制图] 全局 Actor 类复制参数注册完成：角色30Hz、投射物20Hz、PlayerState10Hz、默认取CDO。"));
}

void UDBAReplicationGraph::InitGlobalGraphNodes()
{
	Super::InitGlobalGraphNodes();

	// 创建空间分区网格节点，cell size 10000x10000
	GridNode = CreateNewNode<UReplicationGraphNode_GridSpatialization2D>();
	GridNode->CellSize = 10000.0f;
	GridNode->SpatialBias = FVector2D(-UE_OLD_WORLD_MAX, -UE_OLD_WORLD_MAX);
	AddGlobalGraphNode(GridNode);

	// 创建全局相关 Actor 节点（PlayerState、GameMode、RPC Handler 等）
	AlwaysRelevantNode = CreateNewNode<UReplicationGraphNode_ActorList>();
	AddGlobalGraphNode(AlwaysRelevantNode);

	// 创建静态空间节点（用于不移动的 Actor，暂不注册为全局节点，按需使用）
	StaticSpatialNode = CreateNewNode<UReplicationGraphNode_ActorList>();

	UE_LOG(LogDBANetwork, Log, TEXT("[复制图] 全局图初始化完成：网格节点(cell=10000)、全局相关节点已注册，静态空间节点已创建。"));
}

void UDBAReplicationGraph::RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalInfo)
{
	AActor* Actor = ActorInfo.Actor;
	if (!Actor)
	{
		return;
	}

	// PlayerState、GameMode、RPC Handler（bAlwaysRelevant）→ AlwaysRelevantNode
	if (Actor->bAlwaysRelevant
		|| Actor->IsA(APlayerState::StaticClass())
		|| Actor->IsA(AGameModeBase::StaticClass()))
	{
		AlwaysRelevantNode->NotifyAddNetworkActor(ActorInfo);
		return;
	}

	// 角色(Pawn/Character)、投射物及其他空间相关 Actor → GridNode
	// 使用 Dormancy 模式，让网格节点自动处理静态/动态切换
	GridNode->AddActor_Dormancy(ActorInfo, GlobalInfo);
}

void UDBAReplicationGraph::RouteRemoveNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo)
{
	AActor* Actor = ActorInfo.Actor;
	if (!Actor)
	{
		return;
	}

	// 根据添加时的路由策略，从对应节点移除
	if (Actor->bAlwaysRelevant
		|| Actor->IsA(APlayerState::StaticClass())
		|| Actor->IsA(AGameModeBase::StaticClass()))
	{
		AlwaysRelevantNode->NotifyRemoveNetworkActor(ActorInfo);
	}
	else
	{
		GridNode->RemoveActor_Dormancy(ActorInfo);
	}
}
