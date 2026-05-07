// Copyright Freebooz Games, Inc. All Rights Reserved.
// 怪物AI控制器实现

#include "GameDBA/Character/Monster/AI/DBAMonsterAIController.h"
#include "GameDBA/Character/Monster/AI/DBAMonsterAIComponent.h"
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