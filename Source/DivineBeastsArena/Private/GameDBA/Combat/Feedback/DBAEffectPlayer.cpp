// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Combat/Feedback/DBAEffectPlayer.h"

#include "Camera/PlayerCameraManager.h"
#include "GameDBA/Combat/Feedback/DBAEffectTableManager.h"
#include "GameDBA/Combat/Feedback/DBAFloatingDamageComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

UDBAEffectPlayer::UDBAEffectPlayer() {}

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
	return World ? World->GetSubsystem<UDBAEffectPlayer>() : nullptr;
}

void UDBAEffectPlayer::PlayReleaseEffect(AActor* Caster, FName SkillID, FVector Location, FRotator Direction)
{
	FDBASkillEffectRow EffectData;
	if (GetSkillEffectData(SkillID, EffectData))
	{
		SafeSpawnNiagaraEffect(EffectData.ReleaseEffect.Get(), Location, Direction);
	}
}

void UDBAEffectPlayer::PlayHitEffect(AActor* Target, FName SkillID, FVector ImpactPoint)
{
	FDBASkillEffectRow EffectData;
	if (GetSkillEffectData(SkillID, EffectData))
	{
		SafeSpawnNiagaraEffect(EffectData.HitEffect.Get(), ImpactPoint, FRotator::ZeroRotator);
	}
}

void UDBAEffectPlayer::PlaySound(AActor* Target, FName SkillID, bool bIsHit)
{
	FDBASkillEffectRow EffectData;
	if (!GetSkillEffectData(SkillID, EffectData))
	{
		return;
	}

	USoundBase* Sound = bIsHit ? EffectData.HitSound.Get() : EffectData.CastSound.Get();
	if (Sound)
	{
		SafePlaySound(Sound, Target ? Target->GetActorLocation() : FVector::ZeroVector, true);
	}
}

void UDBAEffectPlayer::TriggerScreenShake(AActor* Target, FName SkillID, float Scale)
{
	FDBASkillEffectRow EffectData;
	if (!GetSkillEffectData(SkillID, EffectData) || !EffectData.ScreenShakeClass)
	{
		return;
	}

	APlayerController* PC = nullptr;
	if (const ACharacter* Character = Cast<ACharacter>(Target))
	{
		PC = Character->GetController<APlayerController>();
	}
	else if (const APawn* Pawn = Cast<APawn>(Target))
	{
		PC = Pawn->GetController<APlayerController>();
	}
	if (!PC && GetWorld())
	{
		PC = GetWorld()->GetFirstPlayerController();
	}
	if (PC && PC->PlayerCameraManager)
	{
		PC->PlayerCameraManager->StartCameraShake(EffectData.ScreenShakeClass, EffectData.ShakeScale * Scale);
	}
}

void UDBAEffectPlayer::SpawnDamageNumber(AActor* Target, float Damage, bool bIsCritical, uint8 ElementValue, FVector ImpactPoint)
{
	if (UDBAFloatingDamageComponent* DamageComponent = Target ? Target->FindComponentByClass<UDBAFloatingDamageComponent>() : nullptr)
	{
		DamageComponent->SpawnDamageNumber(Damage, bIsCritical, ElementValue, ImpactPoint);
	}
}

void UDBAEffectPlayer::SetEffectTableManager(UDBAEffectTableManager* Manager)
{
	EffectTableManager = Manager;
}

bool UDBAEffectPlayer::GetSkillEffectData(FName SkillID, FDBASkillEffectRow& OutEffectData) const
{
	if (!EffectTableManager.IsValid())
	{
		return false;
	}
	OutEffectData = EffectTableManager->GetSkillEffect(SkillID);
	return OutEffectData.IsLoaded() || OutEffectData.ScreenShakeClass != nullptr;
}

UNiagaraComponent* UDBAEffectPlayer::SafeSpawnNiagaraEffect(UNiagaraSystem* System, FVector Location, FRotator Rotation)
{
	if (!System || !GetWorld())
	{
		return nullptr;
	}
	return UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), System, Location, Rotation, FVector(1.0f), true, true, ENCPoolMethod::AutoRelease, true);
}

void UDBAEffectPlayer::SafePlaySound(USoundBase* SoundBase, FVector Location, bool bIs3D)
{
	if (!SoundBase || !GetWorld())
	{
		return;
	}
	if (bIs3D)
	{
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), SoundBase, Location);
	}
	else
	{
		UGameplayStatics::SpawnSound2D(GetWorld(), SoundBase);
	}
}
