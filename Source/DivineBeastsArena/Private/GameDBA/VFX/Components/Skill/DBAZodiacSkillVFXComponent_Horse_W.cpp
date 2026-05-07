// Copyright Freebooz Games, Inc. All Rights Reserved.
// 鎶€鑳絍FX/SFX鎸傝浇缁勪欢 - 韪忛澶╅┕鎶€鑳絎

#include "GameDBA/VFX/Components/Skill/DBAZodiacSkillVFXComponent_Horse_W.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"

UDBAZodiacSkillVFXComponent_Horse_W::UDBAZodiacSkillVFXComponent_Horse_W()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UDBAZodiacSkillVFXComponent_Horse_W::PlayCastingVFX(AActor* Target)
{
	if (UParticleSystem* VFX = CastingVFX.LoadSynchronous())
	{
		FVector Location = Target ? Target->GetActorLocation() : GetOwner()->GetActorLocation();
		FRotator Rotation = GetOwner()->GetActorRotation();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, Location, Rotation, true);
	}

	if (USoundBase* SFX = CastingSFX.LoadSynchronous())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, GetOwner()->GetActorLocation());
	}

	// 鎾斁鏂芥硶鍔ㄧ敾
	if (UAnimMontage* Montage = CastingMontage.LoadSynchronous())
	{
		if (USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
		{
			Mesh->GetAnimInstance()->Montage_Play(Montage);
		}
	}
}

void UDBAZodiacSkillVFXComponent_Horse_W::PlayImpactVFX(AActor* HitTarget)
{
	if (UParticleSystem* VFX = ImpactVFX.LoadSynchronous())
	{
		FVector Location = HitTarget ? HitTarget->GetActorLocation() : GetOwner()->GetActorLocation();
		FRotator Rotation = FRotator::ZeroRotator;
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, Location, Rotation, true);
	}

	if (USoundBase* SFX = ImpactSFX.LoadSynchronous())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, GetOwner()->GetActorLocation());
	}

	// 鎾斁鍛戒腑鍔ㄧ敾
	if (UAnimMontage* Montage = ImpactMontage.LoadSynchronous())
	{
		if (USkeletalMeshComponent* Mesh = HitTarget->FindComponentByClass<USkeletalMeshComponent>())
		{
			Mesh->GetAnimInstance()->Montage_Play(Montage);
		}
	}
}

void UDBAZodiacSkillVFXComponent_Horse_W::PlayProjectileVFX(FVector Start, FVector End)
{
	if (UParticleSystem* VFX = ProjectileVFX.LoadSynchronous())
	{
		// 璁＄畻鏂瑰悜
		FVector Direction = (End - Start).GetSafeNormal();
		FRotator Rotation = Direction.Rotation();

		// 鍦ㄨ捣鐐圭敓鎴愰琛岀壒鏁?		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, Start, Rotation, true);
	}

	if (USoundBase* SFX = ProjectileSFX.LoadSynchronous())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, Start);
	}
}

void UDBAZodiacSkillVFXComponent_Horse_W::PlayAOEVFX(FVector Center, float Radius)
{
	if (UParticleSystem* VFX = AOEVFX.LoadSynchronous())
	{
		FRotator Rotation = FRotator::ZeroRotator;
		UParticleSystemComponent* PSystem = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, Center, Rotation, true);
		if (PSystem)
		{
			// 缂╂斁鐗规晥鑼冨洿
			PSystem->SetVectorParameter(FName(TEXT("Radius")), FVector(Radius));
		}
	}
}

void UDBAZodiacSkillVFXComponent_Horse_W::PlayChannelVFX()
{
	if (UParticleSystem* VFX = ChannelVFX.LoadSynchronous())
	{
		FVector Location = GetOwner()->GetActorLocation();
		FRotator Rotation = GetOwner()->GetActorRotation();
		ChannelVFXComponent = UGameplayStatics::SpawnEmitterAttached(
			VFX, GetOwner()->GetRootComponent(), NAME_None, Location, Rotation, EAttachLocation::KeepRelativeOffset, true);
	}

	if (USoundBase* SFX = CastingSFX.LoadSynchronous())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, GetOwner()->GetActorLocation());
	}
}

void UDBAZodiacSkillVFXComponent_Horse_W::StopChannelVFX()
{
	if (ChannelVFXComponent)
	{
		ChannelVFXComponent->Deactivate();
		ChannelVFXComponent = nullptr;
	}
}

void UDBAZodiacSkillVFXComponent_Horse_W::PlayProjectileSFX()
{
	if (USoundBase* SFX = ProjectileSFX.LoadSynchronous())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, GetOwner()->GetActorLocation());
	}
}

void UDBAZodiacSkillVFXComponent_Horse_W::PlayImpactSFX()
{
	if (USoundBase* SFX = ImpactSFX.LoadSynchronous())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, GetOwner()->GetActorLocation());
	}
}


void UDBAZodiacSkillVFXComponent_Horse_W::PlayCastingSFX()
{
	if (USoundBase* SFX = CastingSFX.LoadSynchronous())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector);
	}
}
