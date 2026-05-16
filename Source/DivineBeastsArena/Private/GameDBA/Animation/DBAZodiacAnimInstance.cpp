// Copyright Freebooz Games, Inc. All Rights Reserved.

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
