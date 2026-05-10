// Copyright Freebooz Games, Inc. All Rights Reserved.
// 泛化生肖VFX组件实现

#include "GameDBA/VFX/Components/DBAZodiacVFXComponent_Generic.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/Core/DBAConstants.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"

UDBAZodiacVFXComponent_Generic::UDBAZodiacVFXComponent_Generic()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDBAZodiacVFXComponent_Generic::LoadDefaultAssets()
{
	UE_LOG(LogDBACombat, Log, TEXT("[DBAZodiacVFXComponent_Generic] 加载默认资源: ZodiacType=%d"), (uint8)ZodiacType);
}

void UDBAZodiacVFXComponent_Generic::PlayAttackVFX(AActor* Target)
{
	if (UParticleSystem* VFX = AttackVFX.LoadSynchronous())
	{
		FVector Location = Target ? Target->GetActorLocation() : GetOwner()->GetActorLocation();
		FRotator Rotation = GetOwner()->GetActorRotation();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, Location, Rotation, true);
	}
}

void UDBAZodiacVFXComponent_Generic::PlayHitVFX(AActor* Attacker)
{
	if (UParticleSystem* VFX = HitVFX.LoadSynchronous())
	{
		FVector Location = GetOwner()->GetActorLocation();
		FRotator Rotation = FRotator::ZeroRotator;
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, Location, Rotation, true);
	}
}

void UDBAZodiacVFXComponent_Generic::PlayMoveVFX(const FVector& Direction)
{
	if (UParticleSystem* VFX = MoveVFX.LoadSynchronous())
	{
		FVector Location = GetOwner()->GetActorLocation();
		FRotator Rotation = Direction.Rotation();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, Location, Rotation, true);
	}
}

void UDBAZodiacVFXComponent_Generic::PlayDeathVFX()
{
	if (UParticleSystem* VFX = DeathVFX.LoadSynchronous())
	{
		FVector Location = GetOwner()->GetActorLocation();
		FRotator Rotation = FRotator::ZeroRotator;
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, Location, Rotation, true);
	}
}

void UDBAZodiacVFXComponent_Generic::PlayRespawnVFX()
{
	if (UParticleSystem* VFX = RespawnVFX.LoadSynchronous())
	{
		FVector Location = GetOwner()->GetActorLocation();
		FRotator Rotation = FRotator::ZeroRotator;
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, Location, Rotation, true);
	}
}

void UDBAZodiacVFXComponent_Generic::PlayAttackSFX()
{
	if (USoundBase* SFX = AttackSFX.LoadSynchronous())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, GetOwner()->GetActorLocation());
	}
}

void UDBAZodiacVFXComponent_Generic::PlayHitSFX()
{
	if (USoundBase* SFX = HitSFX.LoadSynchronous())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, GetOwner()->GetActorLocation());
	}
}

void UDBAZodiacVFXComponent_Generic::PlayMoveSFX()
{
	if (USoundBase* SFX = MoveSFX.LoadSynchronous())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, GetOwner()->GetActorLocation());
	}
}

void UDBAZodiacVFXComponent_Generic::PlayDeathSFX()
{
	if (USoundBase* SFX = DeathSFX.LoadSynchronous())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, GetOwner()->GetActorLocation());
	}
}

void UDBAZodiacVFXComponent_Generic::PlaySkillSFX(FName SkillId)
{
	UE_LOG(LogDBACombat, Verbose, TEXT("[DBAZodiacVFXComponent_Generic] 播放技能音效: %s"), *SkillId.ToString());
}

void UDBAZodiacVFXComponent_Generic::PlayAttackAnimation()
{
	if (UAnimMontage* Montage = AttackMontage.LoadSynchronous())
	{
		if (USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
		{
			Mesh->GetAnimInstance()->Montage_Play(Montage);
		}
	}
}

void UDBAZodiacVFXComponent_Generic::PlayHitAnimation()
{
	if (UAnimMontage* Montage = HitMontage.LoadSynchronous())
	{
		if (USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
		{
			Mesh->GetAnimInstance()->Montage_Play(Montage);
		}
	}
}

void UDBAZodiacVFXComponent_Generic::PlayMoveAnimation(float Speed)
{
	if (USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
	{
		Mesh->SetAnimationSpeedMultiplier(Speed / DBAConstants::AnimationSpeedBase);
	}
}

void UDBAZodiacVFXComponent_Generic::PlayDeathAnimation()
{
	if (UAnimMontage* Montage = DeathMontage.LoadSynchronous())
	{
		if (USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
		{
			Mesh->GetAnimInstance()->Montage_Play(Montage);
		}
	}
}

void UDBAZodiacVFXComponent_Generic::PlaySkillAnimation(FName SkillId)
{
	if (TSoftObjectPtr<UAnimMontage>* MontagePtr = SkillMontages.Find(SkillId))
	{
		if (UAnimMontage* Montage = MontagePtr->LoadSynchronous())
		{
			if (USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
			{
				Mesh->GetAnimInstance()->Montage_Play(Montage);
			}
		}
	}
}

UAnimBlueprint* UDBAZodiacVFXComponent_Generic::GetAnimBlueprint() const
{
	if (UClass* BPClass = AnimBlueprintClass.Load())
	{
		return Cast<UAnimBlueprint>(BPClass->GetDefaultObject());
	}
	return nullptr;
}