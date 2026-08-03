// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// 怪物AI控制器实现

#include "GameDBA/Characters/Monster/AI/DBAMonsterAIController.h"
#include "GameDBA/Characters/Monster/AI/DBAMonsterAIComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"

ADBAMonsterAIController::ADBAMonsterAIController()
{
	// AI控制器不需要Tick
}

void ADBAMonsterAIController::BeginPlay()
{
	Super::BeginPlay();
	InitializeAIComponent();
}

void ADBAMonsterAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// 设置出生点
	if (UDBAMonsterAIComponent* AIComp = GetAIComponent())
	{
		AIComp->SetSpawnLocation(InPawn->GetActorLocation());
	}

	// 运行行为树
	if (BehaviorTree)
	{
		UBlackboardComponent* BBComp = GetBlackboardComponent();
		UseBlackboard(BlackboardAsset, BBComp);
		RunBehaviorTree(BehaviorTree);
	}
}

void ADBAMonsterAIController::OnUnPossess()
{
	Super::OnUnPossess();
}

void ADBAMonsterAIController::InitializeAIComponent()
{
	// 从拥有的Pawn获取AI组件
	if (APawn* OwningPawn = GetPawn())
	{
		AIComponent = Cast<UDBAMonsterAIComponent>(
			OwningPawn->GetComponentByClass(UDBAMonsterAIComponent::StaticClass()));
	}
}