// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
Readable notes:
- App: DBA_GameClient Unreal Engine client.
- Purpose: original chain lightning spell actor with sequenced electric arc jumps.
- Visuals are built from project-owned Niagara assets and generic runtime placement.
*/

#include "GameDBA/Combat/DBAChainLightningSpell.h"

#include "Components/SceneComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameDBA/Combat/DBADamageCalculator.h"
#include "GameDBA/Combat/DBAPlayableSkillTypes.h"
#include "GameDBA/Utilities/DBAAsyncAssetLoader.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

namespace
{
	FVector ChainLightningSocketLocation(AActor* Actor)
	{
		return Actor ? Actor->GetActorLocation() + FVector(0.0f, 0.0f, 72.0f) : FVector::ZeroVector;
	}

	FVector HorizontalForward(AActor* Actor)
	{
		FVector Forward = Actor ? Actor->GetActorForwardVector() : FVector::ForwardVector;
		Forward.Z = 0.0f;
		Forward = Forward.GetSafeNormal();
		return Forward.IsNearlyZero() ? FVector::ForwardVector : Forward;
	}

	FVector MakeParallelTarget(const FVector& Source, const FVector& RawTarget)
	{
		FVector Target = RawTarget;
		Target.Z = Source.Z;
		return Target;
	}

	FRotator RotationBetweenPoints(const FVector& Source, const FVector& Target)
	{
		const FVector Direction = (Target - Source).GetSafeNormal();
		return Direction.IsNearlyZero() ? FRotator::ZeroRotator : Direction.Rotation();
	}

	bool IsUsableChainWorld(const UWorld* World)
	{
		return IsValid(World);
	}
}

ADBAChainLightningSpell::ADBAChainLightningSpell()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	InitialLifeSpan = 3.0f;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	ArcVFXAsset = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/ProjectileHitVFX/NS/NS_ThunderBolt.NS_ThunderBolt")));
	BranchVFXAsset = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/ProjectileHitVFX/NS/NS_Hit_Eletric_01.NS_Hit_Eletric_01")));
	ImpactVFXAsset = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/ProjectileHitVFX/NS/NS_Hit_Thunder.NS_Hit_Thunder")));
	CastSFXAsset = TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Game/DBA/Audio/SFX/Downloaded/Magic/SFX_ChainLightning_PreCast.SFX_ChainLightning_PreCast")));
	FlightSFXAsset = TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Game/DBA/Audio/SFX/Downloaded/Magic/SFX_ChainLightning_Flight.SFX_ChainLightning_Flight")));
	ImpactSFXAsset = TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Game/DBA/Audio/SFX/Downloaded/Magic/SFX_ChainLightning_Impact.SFX_ChainLightning_Impact")));
}

void ADBAChainLightningSpell::ConfigureFromSkillSpec(const FDBAPlayableSkillRuntimeSpec& Spec)
{
	if (Spec.Magnitude > 0.0f)
	{
		BaseDamage = Spec.Magnitude;
	}
	DamageElement = Spec.Element == EDBAElement::None ? DamageElement : Spec.Element;
	ImpactCueTag = Spec.ImpactCueTag;
	NiagaraParameters = Spec.NiagaraParameters;
	if (NiagaraParameters.EffectRadius > 0.0f)
	{
		JumpRadius = NiagaraParameters.EffectRadius;
	}
	if (NiagaraParameters.TickInterval > 0.0f)
	{
		SegmentDelay = NiagaraParameters.TickInterval;
	}
	if (!Spec.CastNiagaraVFXAsset.IsNull())
	{
		BranchVFXAsset = Spec.CastNiagaraVFXAsset;
	}
	if (!Spec.ProjectileNiagaraVFXAsset.IsNull())
	{
		ArcVFXAsset = Spec.ProjectileNiagaraVFXAsset;
	}
	if (!Spec.ImpactNiagaraVFXAsset.IsNull())
	{
		ImpactVFXAsset = Spec.ImpactNiagaraVFXAsset;
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
		ImpactSFXAsset = Spec.ImpactSFXAsset;
	}
}

void ADBAChainLightningSpell::CastChainLightning(AActor* InCaster, AActor* InitialTarget)
{
	UWorld* World = GetWorld();
	if (!World || !InCaster)
	{
		return;
	}

	if (!HasAuthority() && GetNetMode() != NM_Standalone)
	{
		return;
	}

	AActor* CurrentTarget = InitialTarget ? InitialTarget : FindInitialTarget(InCaster);

	TArray<FVector_NetQuantize> Sources;
	TArray<FVector_NetQuantize> Targets;
	TArray<float> SegmentScales;
	TSet<TObjectPtr<AActor>> AlreadyHit;

	FVector CurrentSource = ChainLightningSocketLocation(InCaster);
	if (!CurrentTarget || CurrentTarget == InCaster)
	{
		const FVector FallbackTarget = CurrentSource + HorizontalForward(InCaster) * FMath::Max(JumpRadius * 0.78f, 420.0f);
		Sources.Add(FVector_NetQuantize(CurrentSource));
		Targets.Add(FVector_NetQuantize(MakeParallelTarget(CurrentSource, FallbackTarget)));
		SegmentScales.Add(1.0f);
		CurrentTarget = nullptr;
	}

	for (int32 JumpIndex = 0; JumpIndex < MaxJumps && CurrentTarget; ++JumpIndex)
	{
		const FVector CurrentTargetLocation = MakeParallelTarget(CurrentSource, ChainLightningSocketLocation(CurrentTarget));
		const float SegmentScale = FMath::Pow(FMath::Clamp(DamageFalloffPerJump, 0.0f, 1.0f), JumpIndex);
		Sources.Add(FVector_NetQuantize(CurrentSource));
		Targets.Add(FVector_NetQuantize(CurrentTargetLocation));
		SegmentScales.Add(SegmentScale);

		ApplyChainDamage(InCaster, CurrentTarget, CurrentTargetLocation, JumpIndex);
		AlreadyHit.Add(CurrentTarget);

		CurrentSource = CurrentTargetLocation;
		CurrentTarget = FindNextTarget(CurrentTargetLocation, InCaster, AlreadyHit);
	}

	if (Sources.Num() <= 0)
	{
		return;
	}

	PreloadPresentationAssets();
	StartLocalSequence(Sources, Targets, SegmentScales);
	if (GetNetMode() != NM_Standalone)
	{
		MulticastPlayChainLightning(Sources, Targets, SegmentScales);
	}
}

AActor* ADBAChainLightningSpell::FindInitialTarget(AActor* InCaster) const
{
	return InCaster ? FindNextTarget(InCaster->GetActorLocation(), InCaster, TSet<TObjectPtr<AActor>>()) : nullptr;
}

void ADBAChainLightningSpell::MulticastPlayChainLightning_Implementation(
	const TArray<FVector_NetQuantize>& Sources,
	const TArray<FVector_NetQuantize>& Targets,
	const TArray<float>& SegmentScales)
{
	if (HasAuthority())
	{
		return;
	}

	StartLocalSequence(Sources, Targets, SegmentScales);
}

AActor* ADBAChainLightningSpell::FindNextTarget(const FVector& FromLocation, AActor* Caster, const TSet<TObjectPtr<AActor>>& AlreadyHit) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams(FName(TEXT("DBAChainLightningTargetSearch")), false, Caster);
	TArray<FOverlapResult> Overlaps;
	if (!World->OverlapMultiByObjectType(
		Overlaps,
		FromLocation,
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(JumpRadius),
		QueryParams))
	{
		return nullptr;
	}

	AActor* BestTarget = nullptr;
	float BestDistanceSquared = TNumericLimits<float>::Max();
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Candidate = Overlap.GetActor();
		if (!Candidate || Candidate == Caster || Candidate == this || AlreadyHit.Contains(Candidate))
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(FromLocation, Candidate->GetActorLocation());
		if (DistanceSquared < BestDistanceSquared)
		{
			BestDistanceSquared = DistanceSquared;
			BestTarget = Candidate;
		}
	}

	return BestTarget;
}

void ADBAChainLightningSpell::ApplyChainDamage(AActor* Caster, AActor* Target, const FVector& HitLocation, int32 JumpIndex) const
{
	if (!HasAuthority())
	{
		return;
	}

	if (!Caster || !Target || BaseDamage <= 0.0f)
	{
		return;
	}

	const float DamageScale = FMath::Pow(FMath::Clamp(DamageFalloffPerJump, 0.0f, 1.0f), JumpIndex);
	UDBADamageCalculator::ApplyDamageToTargetWithCue(
		Caster,
		Target,
		BaseDamage * DamageScale,
		DamageElement,
		false,
		GetResolvedImpactCueTag(),
		HitLocation);
}

void ADBAChainLightningSpell::StartLocalSequence(
	const TArray<FVector_NetQuantize>& Sources,
	const TArray<FVector_NetQuantize>& Targets,
	const TArray<float>& SegmentScales)
{
	if (GetNetMode() == NM_DedicatedServer || Sources.Num() != Targets.Num())
	{
		return;
	}

	PendingSources.Reset();
	PendingTargets.Reset();
	PendingSegmentScales.Reset();

	for (int32 Index = 0; Index < Sources.Num(); ++Index)
	{
		PendingSources.Add(FVector(Sources[Index]));
		PendingTargets.Add(FVector(Targets[Index]));
		PendingSegmentScales.Add(SegmentScales.IsValidIndex(Index) ? SegmentScales[Index] : 1.0f);
	}

	PendingSegmentIndex = 0;
	if (PendingSources.IsValidIndex(0))
	{
		PlaySFXAtLocation(CastSFXAsset, PendingSources[0], 0.82f);
	}
	PlayNextLocalSegment();
}

void ADBAChainLightningSpell::PlayNextLocalSegment()
{
	if (!PendingSources.IsValidIndex(PendingSegmentIndex) || !PendingTargets.IsValidIndex(PendingSegmentIndex))
	{
		return;
	}

	const float SegmentScale = PendingSegmentScales.IsValidIndex(PendingSegmentIndex) ? PendingSegmentScales[PendingSegmentIndex] : 1.0f;
	SpawnArcSegment(PendingSources[PendingSegmentIndex], PendingTargets[PendingSegmentIndex], SegmentScale);
	SpawnImpactBurst(PendingTargets[PendingSegmentIndex], SegmentScale);

	++PendingSegmentIndex;
	if (PendingSources.IsValidIndex(PendingSegmentIndex))
	{
		GetWorldTimerManager().SetTimer(SequenceTimerHandle, this, &ADBAChainLightningSpell::PlayNextLocalSegment, SegmentDelay, false);
	}
	else if (HasAuthority())
	{
		SetLifeSpan(0.75f);
	}
}

void ADBAChainLightningSpell::SpawnArcSegment(const FVector& Source, const FVector& Target, float SegmentScale) const
{
	const FVector Delta = Target - Source;
	const float Distance = Delta.Size();
	if (Distance <= KINDA_SMALL_NUMBER || GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	const FVector MidPoint = Source + Delta * 0.5f;
	const FRotator Rotation = RotationBetweenPoints(Source, Target);
	const FVector ArcScale(FMath::Max(Distance / 360.0f, 0.35f), 0.75f + SegmentScale * 0.35f, 0.75f + SegmentScale * 0.35f);

	if (UNiagaraSystem* ArcVFX = ArcVFXAsset.Get())
	{
		UNiagaraComponent* SpawnedVFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			ArcVFX,
			MidPoint,
			Rotation,
			ArcScale,
			true,
			true,
			ENCPoolMethod::AutoRelease,
			true);
		UDBANiagaraSkillParameterLibrary::ApplySkillParameters(SpawnedVFX, NiagaraParameters, BaseDamage * SegmentScale, Target, Delta.GetSafeNormal(), Distance, JumpRadius);
	}
	else
	{
		UWorld* World = GetWorld();
		const FDBANiagaraSkillParameters CapturedParameters = NiagaraParameters;
		const float CapturedDamage = BaseDamage * SegmentScale;
		const float CapturedRadius = JumpRadius;
		DBAAsyncAssetLoader::RequestAsyncAsset<UNiagaraSystem>(World, ArcVFXAsset, [World, MidPoint, Rotation, ArcScale, Target, Delta, Distance, CapturedParameters, CapturedDamage, CapturedRadius](UNiagaraSystem* LoadedVFX)
		{
			if (!IsUsableChainWorld(World) || !LoadedVFX)
			{
				return;
			}

			UNiagaraComponent* SpawnedVFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				World,
				LoadedVFX,
				MidPoint,
				Rotation,
				ArcScale,
				true,
				true,
				ENCPoolMethod::AutoRelease,
				true);
			UDBANiagaraSkillParameterLibrary::ApplySkillParameters(SpawnedVFX, CapturedParameters, CapturedDamage, Target, Delta.GetSafeNormal(), Distance, CapturedRadius);
		});
	}

	if (UNiagaraSystem* BranchVFX = BranchVFXAsset.Get())
	{
		const FVector BranchOffset = Rotation.RotateVector(FVector(0.0f, 0.0f, 26.0f + 18.0f * SegmentScale));
		UNiagaraComponent* SpawnedVFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			BranchVFX,
			MidPoint + BranchOffset,
			Rotation + FRotator(0.0f, 18.0f, 0.0f),
			FVector(0.55f + SegmentScale * 0.35f),
			true,
			true,
			ENCPoolMethod::AutoRelease,
			true);
		UDBANiagaraSkillParameterLibrary::ApplySkillParameters(SpawnedVFX, NiagaraParameters, BaseDamage * SegmentScale, Target, Delta.GetSafeNormal(), Distance, JumpRadius);
	}
	else
	{
		UWorld* World = GetWorld();
		const FVector BranchLocation = MidPoint + Rotation.RotateVector(FVector(0.0f, 0.0f, 26.0f + 18.0f * SegmentScale));
		const FRotator BranchRotation = Rotation + FRotator(0.0f, 18.0f, 0.0f);
		const FVector BranchScale(0.55f + SegmentScale * 0.35f);
		const FDBANiagaraSkillParameters CapturedParameters = NiagaraParameters;
		const float CapturedDamage = BaseDamage * SegmentScale;
		const float CapturedRadius = JumpRadius;
		DBAAsyncAssetLoader::RequestAsyncAsset<UNiagaraSystem>(World, BranchVFXAsset, [World, BranchLocation, BranchRotation, BranchScale, Target, Delta, Distance, CapturedParameters, CapturedDamage, CapturedRadius](UNiagaraSystem* LoadedVFX)
		{
			if (!IsUsableChainWorld(World) || !LoadedVFX)
			{
				return;
			}

			UNiagaraComponent* SpawnedVFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				World,
				LoadedVFX,
				BranchLocation,
				BranchRotation,
				BranchScale,
				true,
				true,
				ENCPoolMethod::AutoRelease,
				true);
			UDBANiagaraSkillParameterLibrary::ApplySkillParameters(SpawnedVFX, CapturedParameters, CapturedDamage, Target, Delta.GetSafeNormal(), Distance, CapturedRadius);
		});
	}

	PlaySFXAtLocation(FlightSFXAsset, MidPoint, 0.50f + SegmentScale * 0.14f);
}

void ADBAChainLightningSpell::SpawnImpactBurst(const FVector& Location, float SegmentScale) const
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (UNiagaraSystem* ImpactVFX = ImpactVFXAsset.Get())
	{
		UNiagaraComponent* SpawnedVFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			ImpactVFX,
			Location,
			FRotator::ZeroRotator,
			FVector(0.82f + SegmentScale * 0.42f),
			true,
			true,
			ENCPoolMethod::AutoRelease,
			true);
		UDBANiagaraSkillParameterLibrary::ApplySkillParameters(SpawnedVFX, NiagaraParameters, BaseDamage * SegmentScale, Location, FVector::UpVector, 0.0f, JumpRadius);
	}
	else
	{
		UWorld* World = GetWorld();
		const FVector ImpactScale(0.82f + SegmentScale * 0.42f);
		const FDBANiagaraSkillParameters CapturedParameters = NiagaraParameters;
		const float CapturedDamage = BaseDamage * SegmentScale;
		const float CapturedRadius = JumpRadius;
		DBAAsyncAssetLoader::RequestAsyncAsset<UNiagaraSystem>(World, ImpactVFXAsset, [World, Location, ImpactScale, CapturedParameters, CapturedDamage, CapturedRadius](UNiagaraSystem* LoadedVFX)
		{
			if (!IsUsableChainWorld(World) || !LoadedVFX)
			{
				return;
			}

			UNiagaraComponent* SpawnedVFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				World,
				LoadedVFX,
				Location,
				FRotator::ZeroRotator,
				ImpactScale,
				true,
				true,
				ENCPoolMethod::AutoRelease,
				true);
			UDBANiagaraSkillParameterLibrary::ApplySkillParameters(SpawnedVFX, CapturedParameters, CapturedDamage, Location, FVector::UpVector, 0.0f, CapturedRadius);
		});
	}

	PlaySFXAtLocation(ImpactSFXAsset, Location, 0.76f + SegmentScale * 0.18f);
}

void ADBAChainLightningSpell::PlaySFXAtLocation(const TSoftObjectPtr<USoundBase>& Asset, const FVector& Location, float Volume) const
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
			if (!IsUsableChainWorld(World) || !LoadedSFX)
			{
				return;
			}
			UGameplayStatics::PlaySoundAtLocation(World, LoadedSFX, Location, Volume);
		});
	}
}

FGameplayTag ADBAChainLightningSpell::GetResolvedImpactCueTag() const
{
	if (ImpactCueTag.IsValid())
	{
		return ImpactCueTag;
	}
	return FGameplayTag::RequestGameplayTag(FName(TEXT("GameplayCue.DBA.Skill.Impact")), false);
}

void ADBAChainLightningSpell::PreloadPresentationAssets()
{
	TArray<FSoftObjectPath> Paths;
	DBAAsyncAssetLoader::AddPreloadPath(ArcVFXAsset, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(BranchVFXAsset, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(ImpactVFXAsset, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(CastSFXAsset, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(FlightSFXAsset, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(ImpactSFXAsset, Paths);
	DBAAsyncAssetLoader::RequestAsyncPreload(this, Paths);
}
