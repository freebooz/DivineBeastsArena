// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// 鎬墿AI缁勪欢瀹炵幇

#include "GameDBA/Character/Monster/AI/DBAMonsterAIComponent.h"
#include "GameDBA/Character/Monster/DBAMonsterBase.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "EngineUtils.h"

UDBAMonsterAIComponent::UDBAMonsterAIComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f; // AI Tick 棰戠巼 10Hz

	SetIsReplicatedByDefault(true);
}

void UDBAMonsterAIComponent::InitializeComponent()
{
	Super::InitializeComponent();

	// 鍒濆鍖栧嚭鐢熺偣
	AActor* Owner = GetOwner();
	if (Owner)
	{
		SpawnLocation = Owner->GetActorLocation();
	}
}

void UDBAMonsterAIComponent::UninitializeComponent()
{
	Super::UninitializeComponent();
}

void UDBAMonsterAIComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UDBAMonsterAIComponent, CurrentState);
	DOREPLIFETIME(UDBAMonsterAIComponent, CurrentTarget);
	DOREPLIFETIME(UDBAMonsterAIComponent, AggroList);
}

// ===== 瀵艰埅瀹炵幇 =====

void UDBAMonsterAIComponent::MoveToLocation(FVector Destination)
{
	AAIController* AIController = Cast<AAIController>(GetOwner()->GetInstigatorController());
	if (!AIController)
	{
		AIController = Cast<AAIController>(GetOwner()->GetOwner());
	}

	if (AIController)
	{
		UPathFollowingComponent* PathFollowing = AIController->GetPathFollowingComponent();
		if (PathFollowing)
		{
			FAIMoveRequest MoveRequest;
			MoveRequest.SetGoalLocation(Destination);
			MoveRequest.SetAcceptanceRadius(AcceptanceRadius);
			AIController->MoveTo(MoveRequest);
		}
	}

	// 濡傛灉娌℃湁 AIController锛屼娇鐢?CharacterMovement
	if (!AIController)
	{
		if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
		{
			if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
			{
				UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
				if (NavSys)
				{
					FNavLocation NavLocation;
					if (NavSys->GetRandomPointInNavigableRadius(Destination, 50.0f, NavLocation))
					{
						Movement->RequestDirectMove(NavLocation.Location, true);
					}
				}
			}
		}
	}
}

void UDBAMonsterAIComponent::MoveToActor(AActor* Target)
{
	if (!Target)
	{
		return;
	}

	AAIController* AIController = Cast<AAIController>(GetOwner()->GetInstigatorController());
	if (!AIController)
	{
		AIController = Cast<AAIController>(GetOwner()->GetOwner());
	}

	if (AIController)
	{
		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalActor(Target);
		MoveRequest.SetAcceptanceRadius(AcceptanceRadius);
		AIController->MoveTo(MoveRequest);
	}
}

void UDBAMonsterAIComponent::StopMovement()
{
	if (AAIController* AIController = Cast<AAIController>(GetOwner()->GetInstigatorController()))
	{
		AIController->StopMovement();
	}
	else if (AAIController* AIController2 = Cast<AAIController>(GetOwner()->GetOwner()))
	{
		AIController2->StopMovement();
	}
	else if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
		{
			Movement->Velocity = FVector::ZeroVector;
		}
	}
}

bool UDBAMonsterAIComponent::IsMoving() const
{
	if (AAIController* AIController = Cast<AAIController>(GetOwner()->GetInstigatorController()))
	{
		if (UPathFollowingComponent* PathFollowing = AIController->GetPathFollowingComponent())
		{
			return PathFollowing->GetStatus() != EPathFollowingStatus::Idle;
		}
	}
	else if (AAIController* AIController2 = Cast<AAIController>(GetOwner()->GetOwner()))
	{
		if (UPathFollowingComponent* PathFollowing = AIController2->GetPathFollowingComponent())
		{
			return PathFollowing->GetStatus() != EPathFollowingStatus::Idle;
		}
	}

	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
		{
			return !Movement->Velocity.IsZero();
		}
	}

	return false;
}

// ===== 鐘舵€佽浆鎹?=====

void UDBAMonsterAIComponent::TransitionTo(EMonsterAIState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

	EMonsterAIState OldState = CurrentState;
	CurrentState = NewState;

	// 鏍规嵁鏂扮姸鎬佹洿鏂扮Щ鍔ㄩ€熷害
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr)
	{
		if (NewState == EMonsterAIState::Chase || NewState == EMonsterAIState::Attack)
		{
			Movement->MaxWalkSpeed = ChaseSpeed;
		}
		else if (NewState == EMonsterAIState::Patrol)
		{
			Movement->MaxWalkSpeed = PatrolSpeed;
		}
	}
}

// 鍏朵粬鏂规硶淇濇寔鍘熸湁瀹炵幇...

void UDBAMonsterAIComponent::FindTarget()
{
	AActor* BestTarget = GetTopAggroTarget();
	if (!BestTarget)
	{
		const TArray<AActor*> Targets = FindAllValidTargets();
		BestTarget = Targets.Num() > 0 ? Targets[0] : nullptr;
	}
	CurrentTarget = BestTarget;
	TransitionTo(CurrentTarget ? EMonsterAIState::Chase : EMonsterAIState::Idle);
}

void UDBAMonsterAIComponent::ClearTarget()
{
	CurrentTarget = nullptr;
	TransitionTo(EMonsterAIState::Idle);
}

void UDBAMonsterAIComponent::AttackTarget()
{
	if (!CurrentTarget || !IsInAttackRange())
	{
		return;
	}
	LastAttackTime = GetWorld() ? GetWorld()->GetTimeSeconds() : LastAttackTime;
}

bool UDBAMonsterAIComponent::IsInAttackRange() const
{
	const AActor* Owner = GetOwner();
	return Owner && CurrentTarget && FVector::Dist(Owner->GetActorLocation(), CurrentTarget->GetActorLocation()) <= AttackRange;
}

bool UDBAMonsterAIComponent::IsInDetectionRange() const
{
	const AActor* Owner = GetOwner();
	return Owner && CurrentTarget && FVector::Dist(Owner->GetActorLocation(), CurrentTarget->GetActorLocation()) <= DetectionRadius;
}

void UDBAMonsterAIComponent::AddAggro(AActor* Target, float Amount)
{
	if (!Target || Amount <= 0.0f)
	{
		return;
	}
	for (FAggroInfo& Info : AggroList)
	{
		if (Info.Target == Target)
		{
			Info.AddThreat(Amount, GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f);
			return;
		}
	}
	AggroList.Add(FAggroInfo(Target, Amount, GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f));
}

void UDBAMonsterAIComponent::RemoveAggro(AActor* Target)
{
	AggroList.RemoveAll([Target](const FAggroInfo& Info) { return Info.Target == Target; });
}

AActor* UDBAMonsterAIComponent::GetTopAggroTarget()
{
	AActor* BestTarget = nullptr;
	float BestAggro = -1.0f;
	for (const FAggroInfo& Info : AggroList)
	{
		if (Info.Target.IsValid() && Info.Threat > BestAggro)
		{
			BestAggro = Info.Threat;
			BestTarget = Info.Target.Get();
		}
	}
	return BestTarget;
}

void UDBAMonsterAIComponent::ClearAggroList()
{
	AggroList.Empty();
}

int32 UDBAMonsterAIComponent::GetPatrolPointCount() const
{
	return PatrolPoints_CPP.Num();
}

FVector UDBAMonsterAIComponent::GetPatrolPoint(int32 Index) const
{
	return PatrolPoints_CPP.IsValidIndex(Index) ? PatrolPoints_CPP[Index] : SpawnLocation;
}

FVector UDBAMonsterAIComponent::GetNextPatrolPoint()
{
	if (PatrolPoints_CPP.Num() == 0)
	{
		return SpawnLocation;
	}
	const FVector Result = PatrolPoints_CPP[CurrentPatrolIndex];
	CurrentPatrolIndex = bLoopPatrol ? (CurrentPatrolIndex + 1) % PatrolPoints_CPP.Num() : FMath::Min(CurrentPatrolIndex + 1, PatrolPoints_CPP.Num() - 1);
	return Result;
}

void UDBAMonsterAIComponent::UpdateAggroList()
{
	AggroList.RemoveAll([](const FAggroInfo& Info) { return !Info.Target.IsValid() || Info.Threat <= 0.0f; });
	LastAggroUpdateTime = GetWorld() ? GetWorld()->GetTimeSeconds() : LastAggroUpdateTime;
}

bool UDBAMonsterAIComponent::HasLineOfSightTo(FVector TargetLocation) const
{
	return true;
}

TArray<AActor*> UDBAMonsterAIComponent::FindAllValidTargets() const
{
	TArray<AActor*> Result;
	const AActor* Owner = GetOwner();
	if (!Owner || !GetWorld())
	{
		return Result;
	}
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		AActor* Actor = *It;
		if (Actor && Actor != Owner && FVector::Dist(Owner->GetActorLocation(), Actor->GetActorLocation()) <= DetectionRadius)
		{
			Result.Add(Actor);
		}
	}
	return Result;
}

void UDBAMonsterAIComponent::RefreshAggroTarget()
{
	CurrentTarget = GetTopAggroTarget();
}

void UDBAMonsterAIComponent::OnRep_CurrentState(EMonsterAIState OldState)
{
}

void UDBAMonsterAIComponent::OnRep_CurrentTarget(AActor* OldTarget)
{
}

