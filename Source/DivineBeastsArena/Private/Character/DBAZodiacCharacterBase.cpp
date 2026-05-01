// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖角色模型基类

#include "Character/DBAZodiacCharacterBase.h"
#include "DBA/GAS/DBAAbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"

ADBAZodiacCharacterBase::ADBAZodiacCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// 配置碰撞
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
	GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));
}

void ADBAZodiacCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	// Spawn RPC Handler
	if (HasAuthority() && RpcHandlerClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		// 使用FTransform格式的SpawnActor重载
		FTransform SpawnTransform(FRotator::ZeroRotator, GetActorLocation());
		RpcHandler = GetWorld()->SpawnActor<ADBARpcHandler>(RpcHandlerClass, SpawnTransform, SpawnParams);
		RpcHandler->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
	}
}

void ADBAZodiacCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

UDBAZodiacAnimInstance* ADBAZodiacCharacterBase::GetZodiacAnimInstance() const
{
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		return Cast<UDBAZodiacAnimInstance>(MeshComp->GetAnimInstance());
	}
	return nullptr;
}

UDBAAbilitySystemComponent* ADBAZodiacCharacterBase::GetDBAAbilitySystemComponent() const
{
	// 从拥有者获取AbilitySystemComponent
	if (AActor* Owner = GetOwner())
	{
		return Cast<UDBAAbilitySystemComponent>(Owner->FindComponentByClass<UDBAAbilitySystemComponent>());
	}
	return nullptr;
}

// ==================== Animation 实现 ====================

void ADBAZodiacCharacterBase::PlayAttackAnimation()
{
	if (UDBAZodiacAnimInstance* Anim = GetZodiacAnimInstance())
	{
		Anim->SetIsAttacking(true);
	}
}

void ADBAZodiacCharacterBase::PlayHitAnimation()
{
	if (UDBAZodiacAnimInstance* Anim = GetZodiacAnimInstance())
	{
		Anim->SetIsHit(true);
	}
}

void ADBAZodiacCharacterBase::PlayDeathAnimation()
{
	if (UDBAZodiacAnimInstance* Anim = GetZodiacAnimInstance())
	{
		Anim->SetIsDead(true);
	}
}

void ADBAZodiacCharacterBase::SetAnimMoveSpeed(float Speed)
{
	if (UDBAZodiacAnimInstance* AnimInst = GetZodiacAnimInstance())
	{
		AnimInst->SetMoveSpeed(Speed);
	}
}