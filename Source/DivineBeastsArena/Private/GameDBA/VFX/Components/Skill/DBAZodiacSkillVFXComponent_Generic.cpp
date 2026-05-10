// Copyright Freebooz Games, Inc. All Rights Reserved.
// 泛化技能VFX组件实现

#include "GameDBA/VFX/Components/Skill/DBAZodiacSkillVFXComponent_Generic.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"

UDBAZodiacSkillVFXComponent_Generic::UDBAZodiacSkillVFXComponent_Generic()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDBAZodiacSkillVFXComponent_Generic::LoadFromDataTable()
{
	if (!VFXDataTable || ZodiacType == EDBAZodiac::None || SkillSlot.IsNone())
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[DBAZodiacSkillVFXComponent_Generic] 无效的配置: ZodiacType=%d, SkillSlot=%s"),
			(uint8)ZodiacType, *SkillSlot.ToString());
		return;
	}

	static const FString ContextString = TEXT("DBAZodiacSkillVFXComponent_Generic");
	FName RowName = FName(*FString::Printf(TEXT("%s_%s"), *UEnum::GetValueAsString(ZodiacType), *SkillSlot.ToString()));

	FDBAVFXDataRow* Row = VFXDataTable->FindRow<FDBAVFXDataRow>(RowName, ContextString, false);
	if (Row)
	{
		CachedVFXData = *Row;
		UE_LOG(LogDBACombat, Log, TEXT("[DBAZodiacSkillVFXComponent_Generic] 加载VFX配置: %s"), *RowName.ToString());
	}
	else
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[DBAZodiacSkillVFXComponent_Generic] 未找到VFX配置: %s"), *RowName.ToString());
	}
}

FDBAVFXDataRow* UDBAZodiacSkillVFXComponent_Generic::GetVFXData() const
{
	return const_cast<FDBAVFXDataRow*>(&CachedVFXData);
}

void UDBAZodiacSkillVFXComponent_Generic::PlayCastingVFX(AActor* Target)
{
	if (UParticleSystem* VFX = CachedVFXData.CastingVFX.LoadSynchronous())
	{
		FVector Location = Target ? Target->GetActorLocation() : GetOwner()->GetActorLocation();
		FRotator Rotation = GetOwner()->GetActorRotation();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, Location, Rotation, true);
	}

	if (USoundBase* SFX = CachedVFXData.CastingSFX.LoadSynchronous())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, GetOwner()->GetActorLocation());
	}

	if (UAnimMontage* Montage = CachedVFXData.CastingMontage.LoadSynchronous())
	{
		if (USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
		{
			Mesh->GetAnimInstance()->Montage_Play(Montage);
		}
	}
}

void UDBAZodiacSkillVFXComponent_Generic::PlayImpactVFX(AActor* HitTarget)
{
	if (UParticleSystem* VFX = CachedVFXData.ImpactVFX.LoadSynchronous())
	{
		FVector Location = HitTarget ? HitTarget->GetActorLocation() : GetOwner()->GetActorLocation();
		FRotator Rotation = FRotator::ZeroRotator;
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, Location, Rotation, true);
	}

	if (USoundBase* SFX = CachedVFXData.ImpactSFX.LoadSynchronous())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, GetOwner()->GetActorLocation());
	}

	if (UAnimMontage* Montage = CachedVFXData.ImpactMontage.LoadSynchronous())
	{
		if (HitTarget && USkeletalMeshComponent* Mesh = HitTarget->FindComponentByClass<USkeletalMeshComponent>())
		{
			Mesh->GetAnimInstance()->Montage_Play(Montage);
		}
	}
}

void UDBAZodiacSkillVFXComponent_Generic::PlayProjectileVFX(FVector Start, FVector End)
{
	if (UParticleSystem* VFX = CachedVFXData.ProjectileVFX.LoadSynchronous())
	{
		FVector Direction = (End - Start).GetSafeNormal();
		FRotator Rotation = Direction.Rotation();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, Start, Rotation, true);
	}

	if (USoundBase* SFX = CachedVFXData.ProjectileSFX.LoadSynchronous())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, Start);
	}
}

void UDBAZodiacSkillVFXComponent_Generic::PlayAOEVFX(FVector Center, float Radius)
{
	if (UParticleSystem* VFX = CachedVFXData.AOEVFX.LoadSynchronous())
	{
		FRotator Rotation = FRotator::ZeroRotator;
		UParticleSystemComponent* PSystem = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, Center, Rotation, true);
		if (PSystem)
		{
			PSystem->SetVectorParameter(FName(TEXT("Radius")), FVector(Radius));
		}
	}
}

void UDBAZodiacSkillVFXComponent_Generic::PlayChannelVFX()
{
	if (UParticleSystem* VFX = CachedVFXData.ChannelVFX.LoadSynchronous())
	{
		FVector Location = GetOwner()->GetActorLocation();
		FRotator Rotation = GetOwner()->GetActorRotation();
		ChannelVFXComponent = UGameplayStatics::SpawnEmitterAttached(
			VFX, GetOwner()->GetRootComponent(), NAME_None, Location, Rotation, EAttachLocation::KeepRelativeOffset, true);
	}

	if (USoundBase* SFX = CachedVFXData.CastingSFX.LoadSynchronous())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, GetOwner()->GetActorLocation());
	}
}

void UDBAZodiacSkillVFXComponent_Generic::StopChannelVFX()
{
	if (ChannelVFXComponent)
	{
		ChannelVFXComponent->Deactivate();
		ChannelVFXComponent = nullptr;
	}
}

void UDBAZodiacSkillVFXComponent_Generic::PlayCastingSFX()
{
	if (USoundBase* SFX = CachedVFXData.CastingSFX.LoadSynchronous())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector);
	}
}

void UDBAZodiacSkillVFXComponent_Generic::PlayProjectileSFX()
{
	if (USoundBase* SFX = CachedVFXData.ProjectileSFX.LoadSynchronous())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, GetOwner()->GetActorLocation());
	}
}

void UDBAZodiacSkillVFXComponent_Generic::PlayImpactSFX()
{
	if (USoundBase* SFX = CachedVFXData.ImpactSFX.LoadSynchronous())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, GetOwner()->GetActorLocation());
	}
}