// Copyright Freebooz Games, Inc. All Rights Reserved.
// VFX/SFX 挂载组件 - 曜鸣神鸡

#include "DBA/VFX/Components/DBAZodiacVFXComponent_Rooster.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"

UDBAZodiacVFXComponent_Rooster::UDBAZodiacVFXComponent_Rooster()
{
	PrimaryComponentTick.bCanEverTick = false;

	// 设置元素类型
	ElementType = FName(TEXT("Water"));

	// 加载默认资源路径
	LoadDefaultAssets();
}

void UDBAZodiacVFXComponent_Rooster::LoadDefaultAssets()
{
	// VFX 资源路径 - 设计师需要在UE编辑器中配置实际资源
	// AttackVFX.LoadSynchronous();
	// HitVFX.LoadSynchronous();
	// MoveVFX.LoadSynchronous();
	// DeathVFX.LoadSynchronous();
	// RespawnVFX.LoadSynchronous();

	// SFX 资源路径
	// AttackSFX.LoadSynchronous();
	// HitSFX.LoadSynchronous();
	// MoveSFX.LoadSynchronous();
	// DeathSFX.LoadSynchronous();

	// 动画资源路径
	// AnimBlueprintClass.LoadSynchronous();
	// AttackMontage.LoadSynchronous();
	// HitMontage.LoadSynchronous();
	// DeathMontage.LoadSynchronous();
}

// ==================== VFX 实现 ====================

void UDBAZodiacVFXComponent_Rooster::PlayAttackVFX(AActor* Target)
{
	if (AttackVFX.IsValid())
	{
		UParticleSystem* VFX = AttackVFX.LoadSynchronous();
		if (VFX)
		{
			FVector Location = Target ? Target->GetActorLocation() : GetOwner()->GetActorLocation();
			FRotator Rotation = Target ? Target->GetActorRotation() : FRotator::ZeroRotator;
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, Location, Rotation, true);
		}
	}
}

void UDBAZodiacVFXComponent_Rooster::PlayHitVFX(AActor* Attacker)
{
	if (HitVFX.IsValid())
	{
		UParticleSystem* VFX = HitVFX.LoadSynchronous();
		if (VFX)
		{
			FVector Location = GetOwner()->GetActorLocation();
			FRotator Rotation = Attacker ? (Location - Attacker->GetActorLocation()).Rotation() : FRotator::ZeroRotator;
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, Location, Rotation, true);
		}
	}
}

void UDBAZodiacVFXComponent_Rooster::PlayMoveVFX(const FVector& Direction)
{
	if (MoveVFX.IsValid())
	{
		UParticleSystem* VFX = MoveVFX.LoadSynchronous();
		if (VFX)
		{
			FVector Location = GetOwner()->GetActorLocation();
			FRotator Rotation = Direction.Rotation();
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, Location, Rotation, true);
		}
	}
}

void UDBAZodiacVFXComponent_Rooster::PlayDeathVFX()
{
	if (DeathVFX.IsValid())
	{
		UParticleSystem* VFX = DeathVFX.LoadSynchronous();
		if (VFX)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, GetOwner()->GetActorLocation(), FRotator::ZeroRotator, true);
		}
	}
}

void UDBAZodiacVFXComponent_Rooster::PlayRespawnVFX()
{
	if (RespawnVFX.IsValid())
	{
		UParticleSystem* VFX = RespawnVFX.LoadSynchronous();
		if (VFX)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, GetOwner()->GetActorLocation(), FRotator::ZeroRotator, true);
		}
	}
}

// ==================== SFX 实现 ====================

void UDBAZodiacVFXComponent_Rooster::PlayAttackSFX()
{
	if (AttackSFX.IsValid())
	{
		USoundBase* SFX = AttackSFX.LoadSynchronous();
		if (SFX)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, GetOwner()->GetActorLocation());
		}
	}
}

void UDBAZodiacVFXComponent_Rooster::PlayHitSFX()
{
	if (HitSFX.IsValid())
	{
		USoundBase* SFX = HitSFX.LoadSynchronous();
		if (SFX)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, GetOwner()->GetActorLocation());
		}
	}
}

void UDBAZodiacVFXComponent_Rooster::PlayMoveSFX()
{
	if (MoveSFX.IsValid())
	{
		USoundBase* SFX = MoveSFX.LoadSynchronous();
		if (SFX)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, GetOwner()->GetActorLocation());
		}
	}
}

void UDBAZodiacVFXComponent_Rooster::PlayDeathSFX()
{
	if (DeathSFX.IsValid())
	{
		USoundBase* SFX = DeathSFX.LoadSynchronous();
		if (SFX)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, GetOwner()->GetActorLocation());
		}
	}
}

void UDBAZodiacVFXComponent_Rooster::PlaySkillSFX(FName SkillId)
{
	// 根据技能ID播放对应音效 - 设计师需要在UE编辑器中配置资源
	// FString Key = SkillId.ToString();
	// if (Key.Contains("Passive")) { }
	// else if (Key.Contains("Q")) { }
	// else if (Key.Contains("W")) { }
	// else if (Key.Contains("E")) { }
	// else if (Key.Contains("R")) { }
}

// ==================== 动画实现 ====================

void UDBAZodiacVFXComponent_Rooster::PlayAttackAnimation()
{
	if (USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
	{
		if (UAnimMontage* Montage = AttackMontage.LoadSynchronous())
		{
			Mesh->GetAnimInstance()->Montage_Play(Montage);
		}
	}
}

void UDBAZodiacVFXComponent_Rooster::PlayHitAnimation()
{
	if (USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
	{
		if (UAnimMontage* Montage = HitMontage.LoadSynchronous())
		{
			Mesh->GetAnimInstance()->Montage_Play(Montage);
		}
	}
}

void UDBAZodiacVFXComponent_Rooster::PlayMoveAnimation(float Speed)
{
	if (USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
	{
		if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
		{
			AnimInstance->SetFloatValue(FName(TEXT("Speed")), Speed);
		}
	}
}

void UDBAZodiacVFXComponent_Rooster::PlayDeathAnimation()
{
	if (USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
	{
		if (UAnimMontage* Montage = DeathMontage.LoadSynchronous())
		{
			Mesh->GetAnimInstance()->Montage_Play(Montage);
		}
	}
}

void UDBAZodiacVFXComponent_Rooster::PlaySkillAnimation(FName SkillId)
{
	if (USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
	{
		if (UAnimMontage** MontagePtr = SkillMontages.Find(SkillId))
		{
			if (UAnimMontage* Montage = MontagePtr->LoadSynchronous())
			{
				Mesh->GetAnimInstance()->Montage_Play(Montage);
			}
		}
	}
}

UAnimBlueprint* UDBAZodiacVFXComponent_Rooster::GetAnimBlueprint() const
{
	if (UClass* BPClass = AnimBlueprintClass.LoadSynchronous())
	{
		return Cast<UAnimBlueprint>(BPClass->GetDefaultObject());
	}
	return nullptr;
}
