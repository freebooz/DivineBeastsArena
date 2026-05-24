// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
Readable notes:
- App: DBA_GameClient Unreal Engine client.
- Purpose: original delayed bloom healing spell with nature growth and release visuals.
*/

#include "GameDBA/Combat/DBABloomHealingSpell.h"

#include "AbilitySystemComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameDBA/GAS/Attributes/DBABattleAttributeSet.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

namespace
{
	FVector BloomGroundLocation(AActor* Actor)
	{
		return Actor ? Actor->GetActorLocation() + FVector(0.0f, 0.0f, 8.0f) : FVector::ZeroVector;
	}

	UAbilitySystemComponent* ResolveASC(AActor* Actor)
	{
		if (!Actor)
		{
			return nullptr;
		}

		if (UAbilitySystemComponent* ASC = Actor->FindComponentByClass<UAbilitySystemComponent>())
		{
			return ASC;
		}

		return Actor->GetOwner() ? Actor->GetOwner()->FindComponentByClass<UAbilitySystemComponent>() : nullptr;
	}
}

ADBABloomHealingSpell::ADBABloomHealingSpell()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	InitialLifeSpan = 3.5f;

	SeedVFXAsset = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/DBA/VFX/Abilities/WoodCrane/NS_WoodCrane_Q_HealingSeed_Projectile.NS_WoodCrane_Q_HealingSeed_Projectile")));
	GroveVFXAsset = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/DBA/VFX/Abilities/WoodCrane/NS_WoodCrane_Q_HealingGrove_Area.NS_WoodCrane_Q_HealingGrove_Area")));
	BloomVFXAsset = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/DBA/VFX/Abilities/WoodCrane/NS_WoodCrane_Q_HealingBurst_Impact.NS_WoodCrane_Q_HealingBurst_Impact")));
	PulseVFXAsset = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/DBA/VFX/Common/Impact/NS_Impact_Heal_Burst.NS_Impact_Heal_Burst")));
	CastSFXAsset = TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Game/DBA/Audio/SFX/Downloaded/Magic/SFX_BloomHealing_PreCast.SFX_BloomHealing_PreCast")));
	FlightSFXAsset = TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Game/DBA/Audio/SFX/Downloaded/Magic/SFX_BloomHealing_Flight.SFX_BloomHealing_Flight")));
	BloomSFXAsset = TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Game/DBA/Audio/SFX/Downloaded/Magic/SFX_BloomHealing_Impact.SFX_BloomHealing_Impact")));
}

void ADBABloomHealingSpell::CastBloomHealing(AActor* InCaster, AActor* PreferredTarget)
{
	if (!InCaster || !GetWorld())
	{
		return;
	}

	if (!HasAuthority() && GetNetMode() != NM_Standalone)
	{
		return;
	}

	CachedCaster = InCaster;
	CachedPreferredTarget = PreferredTarget;
	const FVector CastLocation = BloomGroundLocation(PreferredTarget ? PreferredTarget : InCaster);
	MulticastPlayBloomStart(CastLocation);
	if (GetNetMode() == NM_Standalone)
	{
		MulticastPlayBloomStart_Implementation(CastLocation);
	}

	GetWorldTimerManager().SetTimer(BloomTimerHandle, this, &ADBABloomHealingSpell::ReleaseBloom, BloomDelay, false);
}

void ADBABloomHealingSpell::ReleaseBloom()
{
	AActor* Caster = CachedCaster.Get();
	if (!Caster)
	{
		return;
	}

	TArray<AActor*> HealTargets = ResolveHealTargets(Caster, CachedPreferredTarget.Get());
	TArray<FVector_NetQuantize> HealTargetLocations;
	for (AActor* Target : HealTargets)
	{
		if (!Target)
		{
			continue;
		}

		ApplyHealing(Target);
		HealTargetLocations.Add(FVector_NetQuantize(Target->GetActorLocation() + FVector(0.0f, 0.0f, 72.0f)));
	}

	const FVector BloomLocation = BloomGroundLocation(CachedPreferredTarget.Get() ? CachedPreferredTarget.Get() : Caster);
	MulticastPlayBloomRelease(BloomLocation, HealTargetLocations);
	if (GetNetMode() == NM_Standalone)
	{
		MulticastPlayBloomRelease_Implementation(BloomLocation, HealTargetLocations);
	}
}

void ADBABloomHealingSpell::MulticastPlayBloomStart_Implementation(FVector_NetQuantize Location)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	const FVector StartLocation(Location);
	SpawnVFX(GroveVFXAsset, StartLocation, FRotator::ZeroRotator, FVector(0.92f));
	SpawnVFX(SeedVFXAsset, StartLocation + FVector(0.0f, 0.0f, 54.0f), FRotator::ZeroRotator, FVector(0.72f));
	PlaySFX(CastSFXAsset, StartLocation, 0.82f);
	PlaySFX(FlightSFXAsset, StartLocation + FVector(0.0f, 0.0f, 42.0f), 0.66f);
}

void ADBABloomHealingSpell::MulticastPlayBloomRelease_Implementation(FVector_NetQuantize Location, const TArray<FVector_NetQuantize>& HealTargetLocations)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	const FVector BloomLocation(Location);
	SpawnVFX(BloomVFXAsset, BloomLocation + FVector(0.0f, 0.0f, 34.0f), FRotator::ZeroRotator, FVector(1.12f));
	SpawnVFX(PulseVFXAsset, BloomLocation + FVector(0.0f, 0.0f, 76.0f), FRotator::ZeroRotator, FVector(1.28f));
	PlaySFX(BloomSFXAsset, BloomLocation, 0.96f);

	for (const FVector_NetQuantize& TargetLocation : HealTargetLocations)
	{
		SpawnVFX(PulseVFXAsset, FVector(TargetLocation), FRotator::ZeroRotator, FVector(0.72f));
	}
}

void ADBABloomHealingSpell::ApplyHealing(AActor* Target) const
{
	if (!Target || HealAmount <= 0.0f)
	{
		return;
	}

	if (UAbilitySystemComponent* ASC = ResolveASC(Target))
	{
		if (UDBABattleAttributeSet* AttrSet = const_cast<UDBABattleAttributeSet*>(ASC->GetSet<UDBABattleAttributeSet>()))
		{
			const float CurrentHealth = AttrSet->GetCurrentHealth();
			const float MaxHealth = AttrSet->GetMaxHealth();
			AttrSet->SetCurrentHealth(FMath::Clamp(CurrentHealth + HealAmount, 0.0f, MaxHealth));
		}
	}
}

void ADBABloomHealingSpell::SpawnVFX(const TSoftObjectPtr<UNiagaraSystem>& Asset, const FVector& Location, const FRotator& Rotation, const FVector& Scale) const
{
	if (Asset.IsNull() || GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (UNiagaraSystem* VFX = Asset.LoadSynchronous())
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			VFX,
			Location,
			Rotation,
			Scale,
			true,
			true,
			ENCPoolMethod::AutoRelease,
			true);
	}
}

void ADBABloomHealingSpell::PlaySFX(const TSoftObjectPtr<USoundBase>& Asset, const FVector& Location, float Volume) const
{
	if (Asset.IsNull() || GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (USoundBase* SFX = Asset.LoadSynchronous())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, Location, Volume);
	}
}

TArray<AActor*> ADBABloomHealingSpell::ResolveHealTargets(AActor* Caster, AActor* PreferredTarget) const
{
	TArray<AActor*> Targets;
	if (PreferredTarget)
	{
		Targets.Add(PreferredTarget);
	}
	if (Caster && !Targets.Contains(Caster))
	{
		Targets.Add(Caster);
	}

	UWorld* World = GetWorld();
	if (!World || !Caster || Targets.Num() >= MaxHealTargets)
	{
		return Targets;
	}

	const FVector Center = BloomGroundLocation(PreferredTarget ? PreferredTarget : Caster);
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	FCollisionQueryParams QueryParams(FName(TEXT("DBABloomHealingTargetSearch")), false, Caster);

	TArray<FOverlapResult> Overlaps;
	if (!World->OverlapMultiByObjectType(Overlaps, Center, FQuat::Identity, ObjectQueryParams, FCollisionShape::MakeSphere(HealRadius), QueryParams))
	{
		return Targets;
	}

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Candidate = Overlap.GetActor();
		if (!Candidate || Targets.Contains(Candidate))
		{
			continue;
		}

		Targets.Add(Candidate);
		if (Targets.Num() >= MaxHealTargets)
		{
			break;
		}
	}

	return Targets;
}
