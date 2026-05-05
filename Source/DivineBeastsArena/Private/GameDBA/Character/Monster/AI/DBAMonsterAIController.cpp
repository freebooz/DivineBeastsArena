// Copyright Freebooz Games, Inc. All Rights Reserved.
// 怪物AI控制器实现

#include "GameDBA/Character/Monster/AI/DBAMonsterAIController.h"
#include "GameDBA/Character/Monster/AI/DBAMonsterAIComponent.h"
#include "GameFramework/Pawn.h"

ADBAMonsterAIController::ADBAMonsterAIController()
{
	// AI控制器不需要Tick
	bWantsBeginPlay = false;
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
	RunBehaviorTree();
}

void ADBAMonsterAIController::OnUnPossess()
{
	StopBehaviorTree();
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
	if (BehaviorTree && !Brain)
	{
		RunBehaviorTree(BehaviorTree);
	}
}

void ADBAMonsterAIController::StopBehaviorTree()
{
	if (Brain)
	{
		Brain->StopLogic(TEXT("UnPossess"));
	}
}