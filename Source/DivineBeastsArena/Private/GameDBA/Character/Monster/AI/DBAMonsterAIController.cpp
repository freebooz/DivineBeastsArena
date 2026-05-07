// Copyright Freebooz Games, Inc. All Rights Reserved.
// 怪物AI控制器实现

#include "GameDBA/Character/Monster/AI/DBAMonsterAIController.h"
#include "GameDBA/Character/Monster/AI/DBAMonsterAIComponent.h"
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
		UseBlackboard(BlackboardAsset, BlackboardComponent);
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

void ADBAMonsterAIController::RunBehaviorTree()
{
	// RunBehaviorTree 已通过 OnPossess 调用
}

void ADBAMonsterAIController::StopBehaviorTree()
{
	// 使用 AAIController 的 StopLogic 方法停止行为树
	StopLogic(TEXT("UnPossess"));
}