// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Combat/Feedback/DBAEffectPlayer.h"
#include "GameDBA/Combat/Feedback/DBAEffectTableManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"

UDBAEffectPlayer::UDBAEffectPlayer()
{
}

void UDBAEffectPlayer::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	CachedWorld = GetWorld();
}

void UDBAEffectPlayer::Deinitialize()
{
	EffectTableManager.Reset();
	CachedWorld.Reset();
	Super::Deinitialize();
}

UDBAEffectPlayer* UDBAEffectPlayer::Get(UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}

	return World->GetSubsystem<UDBAEffectPlayer>();
}

void UDBAEffectPlayer::PlayReleaseEffect(AActor* Caster, FName SkillID, FVector Location, FRotator Direction)
{
	const FDBASkillEffectRow* EffectData = GetSkillEffectData(SkillID);
	if (!EffectData)
	{
		return;
	}

	// 播放释放特效
	if (EffectData->ReleaseEffect.IsValid())
	{
		TSubclassOf<UNiagaraSystem> ReleaseSystem = EffectData->ReleaseEffect.Get();
		if (ReleaseSystem)
		{
			SafeSpawnNiagaraEffect(ReleaseSystem, Location, Direction);
		}
	}
}

void UDBAEffectPlayer::PlayHitEffect(AActor* Target, FName SkillID, FVector ImpactPoint)
{
	const FDBASkillEffectRow* EffectData = GetSkillEffectData(SkillID);
	if (!EffectData)
	{
		return;
	}

	// 播放命中特效
	if (EffectData->HitEffect.IsValid())
	{
		TSubclassOf<UNiagaraSystem> HitSystem = EffectData->HitEffect.Get();
		if (HitSystem)
		{
			SafeSpawnNiagaraEffect(HitSystem, ImpactPoint, FRotator::ZeroRotator);
		}
	}
}

void UDBAEffectPlayer::PlaySound(AActor* Target, FName SkillID, bool bIsHit)
{
	const FDBASkillEffectRow* EffectData = GetSkillEffectData(SkillID);
	if (!EffectData)
	{
		return;
	}

	USoundBase* Sound = bIsHit ? EffectData->HitSound.Get() : EffectData->CastSound.Get();
	if (Sound)
	{
		FVector Location = Target ? Target->GetActorLocation() : FVector::ZeroVector;
		SafePlaySound(Sound, Location, true);
	}
}

void UDBAEffectPlayer::TriggerScreenShake(AActor* Target, FName SkillID, float Scale)
{
	const FDBASkillEffectRow* EffectData = GetSkillEffectData(SkillID);
	if (!EffectData)
	{
		return;
	}

	if (!EffectData->ScreenShakeClass.Get())
	{
		return;
	}

	APlayerController* PC = nullptr;
	if (Target)
	{
		if (ACharacter* Character = Cast<ACharacter>(Target))
		{
			PC = Character->GetController<APlayerController>();
		}
		else if (APawn* Pawn = Cast<APawn>(Target))
		{
			PC = Pawn->GetController<APlayerController>();
		}
	}

	if (!PC)
	{
		if (UWorld* World = GetWorld())
		{
			PC = World->GetFirstPlayerController();
		}
	}

	if (PC)
	{
		PC->ClientPlayCameraShake(EffectData->ScreenShakeClass.Get(), EffectData->ShakeScale * Scale);
	}
}

void UDBAEffectPlayer::SpawnDamageNumber(AActor* Target, float Damage, bool bIsCritical, uint8 ElementValue, FVector ImpactPoint)
{
	if (!Target)
	{
		return;
	}

	// 尝试获取Target身上的FloatingDamageComponent
	UDBAFloatingDamageComponent* DamageComponent = Target->FindComponentByClass<UDBAFloatingDamageComponent>();
	if (DamageComponent)
	{
		DamageComponent->SpawnDamageNumber(Damage, bIsCritical, ElementValue, ImpactPoint);
		return;
	}

	// 如果Target没有，找世界中的
	if (UWorld* World = GetWorld())
	{
		TArray<UActorComponent*> Components;
		World->GetComponents(UDBAFloatingDamageComponent::StaticClass(), Components);
		for (UActorComponent* Component : Components)
		{
			if (UDBAFloatingDamageComponent* DamageComp = Cast<UDBAFloatingDamageComponent>(Component))
			{
				DamageComp->SpawnDamageNumber(Damage, bIsCritical, ElementValue, ImpactPoint);
				break;
			}
		}
	}
}

void UDBAEffectPlayer::SetEffectTableManager(UDBAEffectTableManager* Manager)
{
	EffectTableManager = Manager;
}

const FDBASkillEffectRow* UDBAEffectPlayer::GetSkillEffectData(FName SkillID) const
{
	if (EffectTableManager.IsValid())
	{
		return EffectTableManager->GetSkillEffect(SkillID);
	}
	return nullptr;
}

UNiagaraComponent* UDBAEffectPlayer::SafeSpawnNiagaraEffect(TSubclassOf<UNiagaraSystem> SystemClass, FVector Location, FRotator Rotation)
{
	if (!SystemClass)
	{
		return nullptr;
	}

	if (UWorld* World = GetWorld())
	{
		return UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			SystemClass,
			Location,
			Rotation,
			FVector(1.0f),
			true,
			true,
			ENCPoolMethod::AutoRelease,
			true
		);
	}
	return nullptr;
}

void UDBAEffectPlayer::SafePlaySound(USoundBase* SoundBase, FVector Location, bool bIs3D)
{
	if (!SoundBase)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (bIs3D)
		{
			UGameplayStatics::SpawnSoundAtLocation(World, SoundBase, Location);
		}
		else
		{
			UGameplayStatics::SpawnSound2D(World, SoundBase);
		}
	}
}