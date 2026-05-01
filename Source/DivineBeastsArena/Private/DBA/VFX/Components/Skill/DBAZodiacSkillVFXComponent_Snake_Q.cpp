// Copyright Freebooz Games, Inc. All Rights Reserved.
// 技能VFX/SFX挂载组件 - 玄花灵蛇技能Q

#include "DBA/VFX/Components/Skill/DBAZodiacSkillVFXComponent_Snake_Q.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"

UDBAZodiacSkillVFXComponent_Snake_Q::UDBAZodiacSkillVFXComponent_Snake_Q()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UDBAZodiacSkillVFXComponent_Snake_Q::PlayCastingVFX(AActor* Target)
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

	// 播放施法动画
	if (UAnimMontage* Montage = CastingMontage.LoadSynchronous())
	{
		if (USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
		{
			Mesh->GetAnimInstance()->Montage_Play(Montage);
		}
	}
}

void UDBAZodiacSkillVFXComponent_Snake_Q::PlayImpactVFX(AActor* HitTarget)
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

	// 播放命中动画
	if (UAnimMontage* Montage = ImpactMontage.LoadSynchronous())
	{
		if (USkeletalMeshComponent* Mesh = HitTarget->FindComponentByClass<USkeletalMeshComponent>())
		{
			Mesh->GetAnimInstance()->Montage_Play(Montage);
		}
	}
}

void UDBAZodiacSkillVFXComponent_Snake_Q::PlayProjectileVFX(FVector Start, FVector End)
{
	if (UParticleSystem* VFX = ProjectileVFX.LoadSynchronous())
	{
		// 计算方向
		FVector Direction = (End - Start).GetSafeNormal();
		FRotator Rotation = Direction.Rotation();

		// 在起点生成飞行特效
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, Start, Rotation, true);
	}

	if (USoundBase* SFX = ProjectileSFX.LoadSynchronous())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, Start);
	}
}

void UDBAZodiacSkillVFXComponent_Snake_Q::PlayAOEVFX(FVector Center, float Radius)
{
	if (UParticleSystem* VFX = AOEVFX.LoadSynchronous())
	{
		FRotator Rotation = FRotator::ZeroRotator;
		UParticleSystemComponent* PSystem = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, Center, Rotation, true);
		if (PSystem)
		{
			// 缩放特效范围
			PSystem->SetVectorParameter(FName(TEXT("Radius")), FVector(Radius));
		}
	}
}

void UDBAZodiacSkillVFXComponent_Snake_Q::PlayChannelVFX()
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
		UGameplayStatics::PlaySoundAttached(SFX, GetOwner()->GetRootComponent());
	}
}

void UDBAZodiacSkillVFXComponent_Snake_Q::StopChannelVFX()
{
	if (ChannelVFXComponent)
	{
		ChannelVFXComponent->Deactivate();
		ChannelVFXComponent = nullptr;
	}
}

void UDBAZodiacSkillVFXComponent_Snake_Q::PlayProjectileSFX()
{
	if (USoundBase* SFX = ProjectileSFX.LoadSynchronous())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, GetOwner()->GetActorLocation());
	}
}

void UDBAZodiacSkillVFXComponent_Snake_Q::PlayImpactSFX()
{
	if (USoundBase* SFX = ImpactSFX.LoadSynchronous())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, GetOwner()->GetActorLocation());
	}
}
