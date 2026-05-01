// Copyright Freebooz Games, Inc. All Rights Reserved.
// 生肖角色模型基类

#include "Character/DBAZodiacCharacterBase.h"
#include "DBA/VFX/Components/DBAZodiacVFXComponent.h"
#include "DBA/GAS/DBAAbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"

ADBAZodiacCharacterBase::ADBAZodiacCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// 创建VFX组件
	VFXComponent = CreateDefaultSubobject<UDBAZodiacVFXComponent>(TEXT("VFXComponent"));

	// 配置碰撞
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
	GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));
}

void ADBAZodiacCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	// 初始化元素类型
	if (UClass* AnimClass = GetMesh()->GetAnimClass())
	{
		// 动画类已设置
	}
}

void ADBAZodiacCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

UDBAZodiacAnimInstance* ADBAZodiacCharacterBase::GetZodiacAnimInstance() const
{
	if (USkeletalMeshComponent* Mesh = GetMesh())
	{
		return Cast<UDBAZodiacAnimInstance>(Mesh->GetAnimInstance());
	}
	return nullptr;
}

UDBAAbilitySystemComponent* ADBAZodiacCharacterBase::GetDBAAbilitySystemComponent() const
{
	return Cast<UDBAAbilitySystemComponent>(GetAbilitySystemComponent());
}

// ==================== VFX 实现 ====================

void ADBAZodiacCharacterBase::PlayAttackVFX(AActor* Target)
{
	if (VFXComponent)
	{
		VFXComponent->PlayAttackVFX(Target);
	}
}

void ADBAZodiacCharacterBase::PlayHitVFX(AActor* Attacker)
{
	if (VFXComponent)
	{
		VFXComponent->PlayHitVFX(Attacker);
	}
}

void ADBAZodiacCharacterBase::PlaySkillVFX(FName SkillId, AActor* Target)
{
	if (VFXComponent)
	{
		// 根据技能ID播放对应特效
		FString SkillStr = SkillId.ToString();
		if (SkillStr.Contains("Passive"))
		{
			// 被动技能特效
		}
		else if (SkillStr.Contains("Q"))
		{
			// Q技能特效
		}
		else if (SkillStr.Contains("W"))
		{
			// W技能特效
		}
		else if (SkillStr.Contains("E"))
		{
			// E技能特效
		}
		else if (SkillStr.Contains("R"))
		{
			// R技能特效
		}
	}
}

// ==================== SFX 实现 ====================

void ADBAZodiacCharacterBase::PlayAttackSFX()
{
	if (VFXComponent)
	{
		VFXComponent->PlayAttackSFX();
	}
}

void ADBAZodiacCharacterBase::PlayHitSFX()
{
	if (VFXComponent)
	{
		VFXComponent->PlayHitSFX();
	}
}

void ADBAZodiacCharacterBase::PlaySkillSFX(FName SkillId)
{
	if (VFXComponent)
	{
		VFXComponent->PlaySkillSFX(SkillId);
	}
}

// ==================== Animation 实现 ====================

void ADBAZodiacCharacterBase::PlayAttackAnimation()
{
	if (VFXComponent)
	{
		VFXComponent->PlayAttackAnimation();
	}
}

void ADBAZodiacCharacterBase::PlayHitAnimation()
{
	if (VFXComponent)
	{
		VFXComponent->PlayHitAnimation();
	}
}

void ADBAZodiacCharacterBase::PlayDeathAnimation()
{
	if (VFXComponent)
	{
		VFXComponent->PlayDeathAnimation();
	}
}

void ADBAZodiacCharacterBase::SetAnimMoveSpeed(float Speed)
{
	if (UDBAZodiacAnimInstance* AnimInst = GetZodiacAnimInstance())
	{
		AnimInst->SetMoveSpeed(Speed);
	}
}
