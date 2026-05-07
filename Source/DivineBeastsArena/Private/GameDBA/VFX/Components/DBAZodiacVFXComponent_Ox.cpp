// Copyright Freebooz Games, Inc. All Rights Reserved.
// VFX/SFX 鎸傝浇缁勪欢 - 闀囧渤绁炵墰

#include "GameDBA/VFX/Components/DBAZodiacVFXComponent_Ox.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameDBA/Animation/DBAZodiacAnimInstance.h"
#include "Engine/World.h"

UDBAZodiacVFXComponent_Ox::UDBAZodiacVFXComponent_Ox()
{
	PrimaryComponentTick.bCanEverTick = false;

	// 璁剧疆鍏冪礌绫诲瀷
	ElementType = FName(TEXT("Water"));

	// 鍔犺浇榛樿璧勬簮璺緞
	LoadDefaultAssets();
}

void UDBAZodiacVFXComponent_Ox::LoadDefaultAssets()
{
	// VFX 璧勬簮璺緞 - 璁捐甯堥渶瑕佸湪UE缂栬緫鍣ㄤ腑閰嶇疆瀹為檯璧勬簮
	// AttackVFX.LoadSynchronous();
	// HitVFX.LoadSynchronous();
	// MoveVFX.LoadSynchronous();
	// DeathVFX.LoadSynchronous();
	// RespawnVFX.LoadSynchronous();

	// SFX 璧勬簮璺緞
	// AttackSFX.LoadSynchronous();
	// HitSFX.LoadSynchronous();
	// MoveSFX.LoadSynchronous();
	// DeathSFX.LoadSynchronous();

	// 鍔ㄧ敾璧勬簮璺緞
	// AnimBlueprintClass.LoadSynchronous();
	// AttackMontage.LoadSynchronous();
	// HitMontage.LoadSynchronous();
	// DeathMontage.LoadSynchronous();
}

// ==================== VFX 瀹炵幇 ====================

void UDBAZodiacVFXComponent_Ox::PlayAttackVFX(AActor* Target)
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

void UDBAZodiacVFXComponent_Ox::PlayHitVFX(AActor* Attacker)
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

void UDBAZodiacVFXComponent_Ox::PlayMoveVFX(const FVector& Direction)
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

void UDBAZodiacVFXComponent_Ox::PlayDeathVFX()
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

void UDBAZodiacVFXComponent_Ox::PlayRespawnVFX()
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

// ==================== SFX 瀹炵幇 ====================

void UDBAZodiacVFXComponent_Ox::PlayAttackSFX()
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

void UDBAZodiacVFXComponent_Ox::PlayHitSFX()
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

void UDBAZodiacVFXComponent_Ox::PlayMoveSFX()
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

void UDBAZodiacVFXComponent_Ox::PlayDeathSFX()
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

void UDBAZodiacVFXComponent_Ox::PlaySkillSFX(FName SkillId)
{
	// 鏍规嵁鎶€鑳絀D鎾斁瀵瑰簲闊虫晥 - 璁捐甯堥渶瑕佸湪UE缂栬緫鍣ㄤ腑閰嶇疆璧勬簮
	// FString Key = SkillId.ToString();
	// if (Key.Contains("Passive")) { }
	// else if (Key.Contains("Q")) { }
	// else if (Key.Contains("W")) { }
	// else if (Key.Contains("E")) { }
	// else if (Key.Contains("R")) { }
}

// ==================== 鍔ㄧ敾瀹炵幇 ====================

void UDBAZodiacVFXComponent_Ox::PlayAttackAnimation()
{
	if (USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
	{
		if (UAnimMontage* Montage = AttackMontage.LoadSynchronous())
		{
			Mesh->GetAnimInstance()->Montage_Play(Montage);
		}
	}
}

void UDBAZodiacVFXComponent_Ox::PlayHitAnimation()
{
	if (USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
	{
		if (UAnimMontage* Montage = HitMontage.LoadSynchronous())
		{
			Mesh->GetAnimInstance()->Montage_Play(Montage);
		}
	}
}

void UDBAZodiacVFXComponent_Ox::PlayMoveAnimation(float Speed)
{
	if (USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
	{
		if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
		{
			if (UDBAZodiacAnimInstance* ZodiacAnimInstance = Cast<UDBAZodiacAnimInstance>(AnimInstance))
			{
				ZodiacAnimInstance->SetMoveSpeed(Speed);
			}
		}
	}
}

void UDBAZodiacVFXComponent_Ox::PlayDeathAnimation()
{
	if (USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
	{
		if (UAnimMontage* Montage = DeathMontage.LoadSynchronous())
		{
			Mesh->GetAnimInstance()->Montage_Play(Montage);
		}
	}
}

void UDBAZodiacVFXComponent_Ox::PlaySkillAnimation(FName SkillId)
{
	if (USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
	{
		if (TSoftObjectPtr<UAnimMontage>* MontagePtr = SkillMontages.Find(SkillId))
		{
			if (UAnimMontage* Montage = MontagePtr->LoadSynchronous())
			{
				Mesh->GetAnimInstance()->Montage_Play(Montage);
			}
		}
	}
}

UAnimBlueprint* UDBAZodiacVFXComponent_Ox::GetAnimBlueprint() const
{
	if (UClass* BPClass = AnimBlueprintClass.LoadSynchronous())
	{
		return Cast<UAnimBlueprint>(BPClass->GetDefaultObject());
	}
	return nullptr;
}


