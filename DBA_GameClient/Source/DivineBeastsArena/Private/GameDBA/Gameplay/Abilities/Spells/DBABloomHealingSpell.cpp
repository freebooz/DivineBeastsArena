// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
Readable notes:
- App: DBA_GameClient Unreal Engine client.
- Purpose: original delayed bloom healing spell with nature growth and release visuals.
*/

#include "GameDBA/Gameplay/Abilities/Spells/DBABloomHealingSpell.h"

#include "AbilitySystemComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameDBA/Gameplay/Loadout/DBAPlayableSkillTypes.h"
#include "GameDBA/Gameplay/Progression/Attributes/DBABattleAttributeSet.h"
#include "GameCore/Async/DBAAsyncAssetLoader.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
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

	FVector BloomFlightSocketLocation(AActor* Actor)
	{
		return Actor ? Actor->GetActorLocation() + FVector(0.0f, 0.0f, 86.0f) : FVector::ZeroVector;
	}

	FVector BloomCastSourceLocation(AActor* Caster, AActor* AnchorActor)
	{
		if (!Caster)
		{
			return BloomFlightSocketLocation(AnchorActor);
		}

		FVector Forward = Caster->GetActorForwardVector();
		Forward.Z = 0.0f;
		Forward = Forward.GetSafeNormal();
		if (Forward.IsNearlyZero())
		{
			Forward = FVector::ForwardVector;
		}
		return BloomFlightSocketLocation(Caster) + Forward * 96.0f;
	}

	FRotator BloomTravelRotation(const FVector& Source, const FVector& Target)
	{
		FVector Direction = Target - Source;
		Direction.Z = 0.0f;
		Direction = Direction.GetSafeNormal();
		return Direction.IsNearlyZero() ? FRotator::ZeroRotator : Direction.Rotation();
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

	bool IsUsableBloomWorld(const UWorld* World)
	{
		return IsValid(World);
	}
}

ADBABloomHealingSpell::ADBABloomHealingSpell()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	InitialLifeSpan = 3.5f;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	SeedVFXAsset = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/DBA/VFX/Abilities/WoodCrane/NS_WoodCrane_Q_HealingSeed_Projectile.NS_WoodCrane_Q_HealingSeed_Projectile")));
	GroveVFXAsset = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/DBA/VFX/Abilities/WoodCrane/NS_WoodCrane_Q_HealingGrove_Area.NS_WoodCrane_Q_HealingGrove_Area")));
	BloomVFXAsset = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/DBA/VFX/Abilities/WoodCrane/NS_WoodCrane_Q_HealingBurst_Impact.NS_WoodCrane_Q_HealingBurst_Impact")));
	PulseVFXAsset = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/DBA/VFX/Common/Impact/NS_Impact_Heal_Burst.NS_Impact_Heal_Burst")));
	CastSFXAsset = TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Game/DBA/Audio/SFX/Downloaded/Magic/SFX_BloomHealing_PreCast.SFX_BloomHealing_PreCast")));
	FlightSFXAsset = TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Game/DBA/Audio/SFX/Downloaded/Magic/SFX_BloomHealing_Flight.SFX_BloomHealing_Flight")));
	BloomSFXAsset = TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Game/DBA/Audio/SFX/Downloaded/Magic/SFX_BloomHealing_Impact.SFX_BloomHealing_Impact")));
}

void ADBABloomHealingSpell::ConfigureFromSkillSpec(const FDBAPlayableSkillRuntimeSpec& Spec)
{
	if (Spec.Magnitude > 0.0f)
	{
		HealAmount = Spec.Magnitude;
	}
	NiagaraParameters = Spec.NiagaraParameters;
	if (NiagaraParameters.EffectRadius > 0.0f)
	{
		HealRadius = NiagaraParameters.EffectRadius;
	}
	if (!Spec.CastNiagaraVFXAsset.IsNull())
	{
		GroveVFXAsset = Spec.CastNiagaraVFXAsset;
	}
	if (!Spec.ProjectileNiagaraVFXAsset.IsNull())
	{
		SeedVFXAsset = Spec.ProjectileNiagaraVFXAsset;
	}
	if (!Spec.ImpactNiagaraVFXAsset.IsNull())
	{
		BloomVFXAsset = Spec.ImpactNiagaraVFXAsset;
		PulseVFXAsset = Spec.ImpactNiagaraVFXAsset;
	}
	if (!Spec.CastSFXAsset.IsNull())
	{
		CastSFXAsset = Spec.CastSFXAsset;
	}
	if (!Spec.FlySFXAsset.IsNull())
	{
		FlightSFXAsset = Spec.FlySFXAsset;
	}
	if (!Spec.ImpactSFXAsset.IsNull())
	{
		BloomSFXAsset = Spec.ImpactSFXAsset;
	}
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
	AActor* AnchorActor = PreferredTarget ? PreferredTarget : InCaster;
	const FVector CastLocation = BloomGroundLocation(AnchorActor);
	const FVector SourceLocation = BloomCastSourceLocation(InCaster, AnchorActor);
	FVector FlightTargetLocation = BloomFlightSocketLocation(AnchorActor);
	FlightTargetLocation.Z = SourceLocation.Z;
	PreloadPresentationAssets();
	MulticastPlayBloomStart(AnchorActor, SourceLocation, FlightTargetLocation, CastLocation);
	if (GetNetMode() == NM_Standalone)
	{
		MulticastPlayBloomStart_Implementation(AnchorActor, SourceLocation, FlightTargetLocation, CastLocation);
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

	SetLifeSpan(1.25f);
}

void ADBABloomHealingSpell::MulticastPlayBloomStart_Implementation(
	AActor* AnchorActor,
	FVector_NetQuantize SourceLocation,
	FVector_NetQuantize FlightTargetLocation,
	FVector_NetQuantize BloomLocation)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	const FVector Source(SourceLocation);
	const FVector FlightTarget(FlightTargetLocation);
	const FVector StartLocation(BloomLocation);
	SpawnTravelVFX(SeedVFXAsset, Source, FlightTarget, 0.72f);
	if (AnchorActor)
	{
		SpawnAttachedVFX(GroveVFXAsset, AnchorActor, FVector(0.0f, 0.0f, 8.0f), FRotator::ZeroRotator, FVector(0.92f));
		SpawnAttachedVFX(SeedVFXAsset, AnchorActor, FVector(0.0f, 0.0f, 62.0f), FRotator::ZeroRotator, FVector(0.72f));
	}
	else
	{
		SpawnVFX(GroveVFXAsset, StartLocation, FRotator::ZeroRotator, FVector(0.92f));
		SpawnVFX(SeedVFXAsset, StartLocation + FVector(0.0f, 0.0f, 54.0f), FRotator::ZeroRotator, FVector(0.72f));
	}
	PlaySFX(CastSFXAsset, Source, 0.82f);
	PlaySFX(FlightSFXAsset, (Source + FlightTarget) * 0.5f, 0.66f);
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

	if (!HasAuthority())
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

	if (UNiagaraSystem* VFX = Asset.Get())
	{
		UNiagaraComponent* SpawnedVFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			VFX,
			Location,
			Rotation,
			Scale,
			true,
			true,
			ENCPoolMethod::AutoRelease,
			true);
		UDBANiagaraSkillParameterLibrary::ApplySkillParameters(SpawnedVFX, NiagaraParameters, HealAmount, Location, Rotation.Vector(), 0.0f, HealRadius);
	}
	else
	{
		UWorld* World = GetWorld();
		const FDBANiagaraSkillParameters CapturedParameters = NiagaraParameters;
		const float CapturedHealAmount = HealAmount;
		const float CapturedHealRadius = HealRadius;
		DBAAsyncAssetLoader::RequestAsyncAsset<UNiagaraSystem>(World, Asset, [World, Location, Rotation, Scale, CapturedParameters, CapturedHealAmount, CapturedHealRadius](UNiagaraSystem* LoadedVFX)
		{
			if (!IsUsableBloomWorld(World) || !LoadedVFX)
			{
				return;
			}

			UNiagaraComponent* SpawnedVFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				World,
				LoadedVFX,
				Location,
				Rotation,
				Scale,
				true,
				true,
				ENCPoolMethod::AutoRelease,
				true);
			UDBANiagaraSkillParameterLibrary::ApplySkillParameters(SpawnedVFX, CapturedParameters, CapturedHealAmount, Location, Rotation.Vector(), 0.0f, CapturedHealRadius);
		});
	}
}

void ADBABloomHealingSpell::SpawnAttachedVFX(const TSoftObjectPtr<UNiagaraSystem>& Asset, AActor* AnchorActor, const FVector& RelativeOffset, const FRotator& Rotation, const FVector& Scale) const
{
	if (Asset.IsNull() || !AnchorActor || !AnchorActor->GetRootComponent() || GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (UNiagaraSystem* VFX = Asset.Get())
	{
		if (!AnchorActor || !AnchorActor->GetRootComponent())
		{
			return;
		}
		UNiagaraComponent* SpawnedVFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
			VFX,
			AnchorActor->GetRootComponent(),
			NAME_None,
			RelativeOffset,
			Rotation,
			Scale,
			EAttachLocation::KeepRelativeOffset,
			true,
			ENCPoolMethod::AutoRelease,
			true);
		UDBANiagaraSkillParameterLibrary::ApplySkillParameters(
			SpawnedVFX,
			NiagaraParameters,
			HealAmount,
			AnchorActor->GetActorLocation(),
			AnchorActor->GetActorForwardVector(),
			0.0f,
			HealRadius);
	}
	else
	{
		TWeakObjectPtr<AActor> WeakAnchor(AnchorActor);
		const FDBANiagaraSkillParameters CapturedParameters = NiagaraParameters;
		const float CapturedHealAmount = HealAmount;
		const float CapturedHealRadius = HealRadius;
		DBAAsyncAssetLoader::RequestAsyncAsset<UNiagaraSystem>(AnchorActor, Asset, [WeakAnchor, RelativeOffset, Rotation, Scale, CapturedParameters, CapturedHealAmount, CapturedHealRadius](UNiagaraSystem* LoadedVFX)
		{
			AActor* StrongAnchor = WeakAnchor.Get();
			if (!StrongAnchor || !StrongAnchor->GetRootComponent() || StrongAnchor->GetNetMode() == NM_DedicatedServer || !LoadedVFX)
			{
				return;
			}

			UNiagaraComponent* SpawnedVFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
				LoadedVFX,
				StrongAnchor->GetRootComponent(),
				NAME_None,
				RelativeOffset,
				Rotation,
				Scale,
				EAttachLocation::KeepRelativeOffset,
				true,
				ENCPoolMethod::AutoRelease,
				true);
			UDBANiagaraSkillParameterLibrary::ApplySkillParameters(
				SpawnedVFX,
				CapturedParameters,
				CapturedHealAmount,
				StrongAnchor->GetActorLocation(),
				StrongAnchor->GetActorForwardVector(),
				0.0f,
				CapturedHealRadius);
		});
	}
}

void ADBABloomHealingSpell::SpawnTravelVFX(const TSoftObjectPtr<UNiagaraSystem>& Asset, const FVector& SourceLocation, const FVector& TargetLocation, float WidthScale) const
{
	if (Asset.IsNull() || GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	FVector HorizontalTarget = TargetLocation;
	HorizontalTarget.Z = SourceLocation.Z;
	const FVector Delta = HorizontalTarget - SourceLocation;
	const float Distance = Delta.Size();
	if (Distance <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	const FVector MidPoint = SourceLocation + Delta * 0.5f;
	const FRotator Rotation = BloomTravelRotation(SourceLocation, HorizontalTarget);
	const FVector Scale(FMath::Max(Distance / 360.0f, 0.34f), WidthScale, WidthScale);

	if (UNiagaraSystem* VFX = Asset.Get())
	{
		UNiagaraComponent* SpawnedVFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			VFX,
			MidPoint,
			Rotation,
			Scale,
			true,
			true,
			ENCPoolMethod::AutoRelease,
			true);
		UDBANiagaraSkillParameterLibrary::ApplySkillParameters(SpawnedVFX, NiagaraParameters, HealAmount, TargetLocation, Delta.GetSafeNormal(), Distance, HealRadius);
	}
	else
	{
		UWorld* World = GetWorld();
		const FDBANiagaraSkillParameters CapturedParameters = NiagaraParameters;
		const float CapturedHealAmount = HealAmount;
		const float CapturedHealRadius = HealRadius;
		DBAAsyncAssetLoader::RequestAsyncAsset<UNiagaraSystem>(World, Asset, [World, MidPoint, Rotation, Scale, TargetLocation, Delta, Distance, CapturedParameters, CapturedHealAmount, CapturedHealRadius](UNiagaraSystem* LoadedVFX)
		{
			if (!IsUsableBloomWorld(World) || !LoadedVFX)
			{
				return;
			}

			UNiagaraComponent* SpawnedVFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				World,
				LoadedVFX,
				MidPoint,
				Rotation,
				Scale,
				true,
				true,
				ENCPoolMethod::AutoRelease,
				true);
			UDBANiagaraSkillParameterLibrary::ApplySkillParameters(SpawnedVFX, CapturedParameters, CapturedHealAmount, TargetLocation, Delta.GetSafeNormal(), Distance, CapturedHealRadius);
		});
	}
}

void ADBABloomHealingSpell::PlaySFX(const TSoftObjectPtr<USoundBase>& Asset, const FVector& Location, float Volume) const
{
	if (Asset.IsNull() || GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (USoundBase* SFX = Asset.Get())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, Location, Volume);
	}
	else
	{
		UWorld* World = GetWorld();
		DBAAsyncAssetLoader::RequestAsyncAsset<USoundBase>(World, Asset, [World, Location, Volume](USoundBase* LoadedSFX)
		{
			if (!IsUsableBloomWorld(World) || !LoadedSFX)
			{
				return;
			}
			UGameplayStatics::PlaySoundAtLocation(World, LoadedSFX, Location, Volume);
		});
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

void ADBABloomHealingSpell::PreloadPresentationAssets()
{
	TArray<FSoftObjectPath> Paths;
	DBAAsyncAssetLoader::AddPreloadPath(SeedVFXAsset, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(GroveVFXAsset, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(BloomVFXAsset, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(PulseVFXAsset, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(CastSFXAsset, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(FlightSFXAsset, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(BloomSFXAsset, Paths);
	DBAAsyncAssetLoader::RequestAsyncPreload(this, Paths);
}
