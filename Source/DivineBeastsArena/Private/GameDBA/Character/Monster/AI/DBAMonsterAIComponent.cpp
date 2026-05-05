// Copyright Freebooz Games, Inc. All Rights Reserved.
// 怪物AI组件实现

#include "GameDBA/Character/Monster/AI/DBAMonsterAIComponent.h"
#include "GameDBA/Character/Monster/DBAMonsterBase.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "GameFramework/CharacterMovementComponent.h"

UDBAMonsterAIComponent::UDBAMonsterAIComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f; // AI Tick 频率 10Hz

	SetIsReplicated(true);
}

void UDBAMonsterAIComponent::InitializeComponent()
{
	Super::InitializeComponent();

	// 初始化出生点
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

// ===== 导航实现 =====

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
			MoveRequest.SetNavAgentLocation(GetOwner()->GetActorLocation());
			AIController->MoveTo(MoveRequest);
		}
	}

	// 如果没有 AIController，使用 CharacterMovement
	if (!AIController)
	{
		if (APawn* Pawn = Cast<APawn>(GetOwner()))
		{
			if (UCharacterMovementComponent* Movement = Pawn->GetCharacterMovement())
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
	else if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		if (UCharacterMovementComponent* Movement = Pawn->GetCharacterMovement())
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
			return PathFollowing->IsMoving();
		}
	}
	else if (AAIController* AIController2 = Cast<AAIController>(GetOwner()->GetOwner()))
	{
		if (UPathFollowingComponent* PathFollowing = AIController2->GetPathFollowingComponent())
		{
			return PathFollowing->IsMoving();
		}
	}

	if (APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		if (UCharacterMovementComponent* Movement = Pawn->GetCharacterMovement())
		{
			return !Movement->Velocity.IsZero();
		}
	}

	return false;
}

// ===== 状态转换 =====

void UDBAMonsterAIComponent::TransitionTo(EMonsterAIState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

	EMonsterAIState OldState = CurrentState;
	CurrentState = NewState;

	// 根据新状态更新移动速度
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (UCharacterMovementComponent* Movement = Pawn ? Pawn->GetCharacterMovement() : nullptr)
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

// 其他方法保持原有实现...