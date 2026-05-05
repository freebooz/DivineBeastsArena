// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖角色模型基类

#include "GameDBA/Character/DBAZodiacCharacterBase.h"
#include "GameDBA/GAS/DBAAbilitySystemComponent.h"
#include "GameDBA/GAS/Attributes/DBABattleAttributeSet.h"
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

// ==================== 属性访问实现 ====================

float ADBAZodiacCharacterBase::GetCurrentHealth() const
{
	if (UDBAAbilitySystemComponent* ASC = GetDBAAbilitySystemComponent())
	{
		return ASC->GetNumericAttributeBase(UDBABattleAttributeSet::GetCurrentHealthAttribute());
	}
	return 0.0f;
}

float ADBAZodiacCharacterBase::GetMaxHealth() const
{
	if (UDBAAbilitySystemComponent* ASC = GetDBAAbilitySystemComponent())
	{
		return ASC->GetNumericAttributeBase(UDBABattleAttributeSet::GetMaxHealthAttribute());
	}
	return 0.0f;
}

float ADBAZodiacCharacterBase::GetCurrentEnergy() const
{
	if (UDBAAbilitySystemComponent* ASC = GetDBAAbilitySystemComponent())
	{
		return ASC->GetNumericAttributeBase(UDBABattleAttributeSet::GetCurrentEnergyAttribute());
	}
	return 0.0f;
}

void ADBAZodiacCharacterBase::SetUltimateEnergy(float Value)
{
	if (HasAuthority())
	{
		UltimateEnergy = FMath::Clamp(Value, 0.0f, 100.0f);
	}
}

void ADBAZodiacCharacterBase::AddUltimateEnergy(float Delta)
{
	if (HasAuthority())
	{
		UltimateEnergy = FMath::Clamp(UltimateEnergy + Delta, 0.0f, 100.0f);
	}
}

void ADBAZodiacCharacterBase::AddChainLevel(int32 Delta)
{
	if (HasAuthority())
	{
		ChainLevel = FMath::Clamp(ChainLevel + Delta, 0, 10);
	}
}

void ADBAZodiacCharacterBase::ResetChainLevel()
{
	if (HasAuthority())
	{
		ChainLevel = 0;
	}
}
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