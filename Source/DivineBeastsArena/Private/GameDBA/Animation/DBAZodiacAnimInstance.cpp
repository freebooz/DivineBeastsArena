// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Animation/DBAZodiacAnimInstance.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"

UDBAZodiacAnimInstance::UDBAZodiacAnimInstance()
{
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
	bIsMoving = GroundSpeed > 3.0f;

	if (const ACharacter* CharacterOwner = Cast<ACharacter>(OwningPawn))
	{
		if (const UCharacterMovementComponent* MovementComponent = CharacterOwner->GetCharacterMovement())
		{
			bIsInAir = MovementComponent->IsFalling();
		}
	}
}
