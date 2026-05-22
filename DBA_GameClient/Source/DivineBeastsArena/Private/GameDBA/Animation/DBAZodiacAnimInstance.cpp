// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Animation/DBAZodiacAnimInstance.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

UDBAZodiacAnimInstance::UDBAZodiacAnimInstance()
{
}

void UDBAZodiacAnimInstance::SetIsAttacking(bool bInAttacking)
{
	bIsAttacking = bInAttacking;
	if (bIsAttacking)
	{
		AttackStateStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : -1.0f;
	}
}

void UDBAZodiacAnimInstance::SetIsHit(bool bInHit)
{
	bIsHit = bInHit;
	if (bIsHit)
	{
		HitStateStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : -1.0f;
	}
}

void UDBAZodiacAnimInstance::SetIsDead(bool bInDead)
{
	bIsDead = bInDead;
	if (bIsDead)
	{
		bIsAttacking = false;
		bIsHit = false;
	}
}

void UDBAZodiacAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwningPawn = TryGetPawnOwner();
}

void UDBAZodiacAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (!OwningPawn)
	{
		OwningPawn = TryGetPawnOwner();
	}

	if (!OwningPawn)
	{
		return;
	}

	const FVector Velocity = OwningPawn->GetVelocity();
	GroundSpeed = FVector(Velocity.X, Velocity.Y, 0.0f).Size();
	MoveSpeed = GroundSpeed;
	Direction = CalculateDirection(Velocity, OwningPawn->GetActorRotation());
	VerticalSpeed = Velocity.Z;
	bIsMoving = GroundSpeed > MoveSpeedThreshold;
	bIsRunning = GroundSpeed > RunSpeedThreshold;

	if (const ACharacter* CharacterOwner = Cast<ACharacter>(OwningPawn))
	{
		if (const UCharacterMovementComponent* MovementComponent = CharacterOwner->GetCharacterMovement())
		{
			bIsInAir = MovementComponent->IsFalling();
			bIsJumping = bIsInAir && VerticalSpeed > JumpVerticalSpeedThreshold;
			bIsFalling = bIsInAir && VerticalSpeed < -FallVerticalSpeedThreshold;
		}
	}
	else
	{
		bIsInAir = false;
		bIsJumping = false;
		bIsFalling = false;
	}

	if (!bIsDead)
	{
		const UWorld* World = GetWorld();
		const float Now = World ? World->GetTimeSeconds() : -1.0f;
		if (bIsAttacking && AttackStateDuration > 0.0f && AttackStateStartTime >= 0.0f && Now >= 0.0f)
		{
			if (Now - AttackStateStartTime >= AttackStateDuration)
			{
				bIsAttacking = false;
				AttackStateStartTime = -1.0f;
			}
		}
		if (bIsHit && HitStateDuration > 0.0f && HitStateStartTime >= 0.0f && Now >= 0.0f)
		{
			if (Now - HitStateStartTime >= HitStateDuration)
			{
				bIsHit = false;
				HitStateStartTime = -1.0f;
			}
		}
	}
}
