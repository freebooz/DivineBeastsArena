// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
Readable notes:
- App: DBA_GameClient Unreal Engine client.
- Purpose: original priest-like protective shield spell with holy barrier VFX and staged SFX.
*/

#include "GameDBA/Combat/DBAHolyShieldSpell.h"

#include "AbilitySystemComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameDBA/Combat/DBAPlayableSkillTypes.h"
#include "GameDBA/GAS/Attributes/DBABattleAttributeSet.h"
#include "GameDBA/Utilities/DBAAsyncAssetLoader.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

namespace
{
	FVector ShieldCenterLocation(AActor* Actor)
	{
		return Actor ? Actor->GetActorLocation() + FVector(0.0f, 0.0f, 74.0f) : FVector::ZeroVector;
	}

	FVector ShieldCastSourceLocation(AActor* Caster, AActor* Target)
	{
		if (!Caster)
		{
			return ShieldCenterLocation(Target);
		}

		FVector Forward = Caster->GetActorForwardVector();
		Forward.Z = 0.0f;
		Forward = Forward.GetSafeNormal();
		if (Forward.IsNearlyZero())
		{
			Forward = FVector::ForwardVector;
		}

		return ShieldCenterLocation(Caster) + Forward * 96.0f;
	}

	FRotator ShieldTravelRotation(const FVector& Source, const FVector& Target)
	{
		FVector Direction = Target - Source;
		Direction.Z = 0.0f;
		Direction = Direction.GetSafeNormal();
		return Direction.IsNearlyZero() ? FRotator::ZeroRotator : Direction.Rotation();
	}

	UAbilitySystemComponent* ResolveHolyShieldASC(AActor* Actor)
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

	bool IsUsableHolyShieldWorld(const UWorld* World)
	{
		return IsValid(World);
	}
}

ADBAHolyShieldSpell::ADBAHolyShieldSpell()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	InitialLifeSpan = 7.0f;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	CastVFXAsset = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/ProjectileHitVFX/NS/NS_Hit_Bless.NS_Hit_Bless")));
	BarrierVFXAsset = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/DBA/VFX/Common/Status/NS_Status_Shielded.NS_Status_Shielded")));
	FlightVFXAsset = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/ProjectileHitVFX/NS/NS_HolyEnergy.NS_HolyEnergy")));
	ImpactVFXAsset = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/ProjectileHitVFX/NS/NS_HolyEnergy.NS_HolyEnergy")));
	CastSFXAsset = TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Game/DBA/Audio/SFX/Downloaded/ClassMagic/SFX_PriestShield_PreCast.SFX_PriestShield_PreCast")));
	SustainSFXAsset = TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Game/DBA/Audio/SFX/Downloaded/ClassMagic/SFX_PriestShield_Flight.SFX_PriestShield_Flight")));
	ImpactSFXAsset = TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Game/DBA/Audio/SFX/Downloaded/ClassMagic/SFX_PriestShield_Impact.SFX_PriestShield_Impact")));
}

void ADBAHolyShieldSpell::ConfigureFromSkillSpec(const FDBAPlayableSkillRuntimeSpec& Spec)
{
	if (Spec.Magnitude > 0.0f)
	{
		ShieldAmount = Spec.Magnitude;
	}
	NiagaraParameters = Spec.NiagaraParameters;
	if (NiagaraParameters.Duration > 0.0f)
	{
		ShieldDuration = NiagaraParameters.Duration;
	}
	if (!Spec.CastNiagaraVFXAsset.IsNull())
	{
		CastVFXAsset = Spec.CastNiagaraVFXAsset;
	}
	if (!Spec.ProjectileNiagaraVFXAsset.IsNull())
	{
		FlightVFXAsset = Spec.ProjectileNiagaraVFXAsset;
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
		SustainSFXAsset = Spec.FlySFXAsset;
	}
	if (!Spec.ImpactSFXAsset.IsNull())
	{
		ImpactSFXAsset = Spec.ImpactSFXAsset;
	}
}

void ADBAHolyShieldSpell::CastHolyShield(AActor* InCaster, AActor* PreferredTarget)
{
	if (!InCaster || !GetWorld())
	{
		return;
	}
	if (!HasAuthority() && GetNetMode() != NM_Standalone)
	{
		return;
	}

	ShieldTarget = PreferredTarget ? PreferredTarget : InCaster;
	ApplyShield(ShieldTarget);

	const FVector Location = ShieldCenterLocation(ShieldTarget);
	const FVector SourceLocation = ShieldCastSourceLocation(InCaster, ShieldTarget);
	FVector FlightTargetLocation = Location;
	FlightTargetLocation.Z = SourceLocation.Z;
	PreloadPresentationAssets();
	MulticastPlayShieldStart(ShieldTarget, SourceLocation, FlightTargetLocation, Location);
	if (GetNetMode() == NM_Standalone)
	{
		MulticastPlayShieldStart_Implementation(ShieldTarget, SourceLocation, FlightTargetLocation, Location);
	}

	GetWorldTimerManager().SetTimer(ShieldTimerHandle, this, &ADBAHolyShieldSpell::ReleaseShield, ShieldDuration, false);
}

void ADBAHolyShieldSpell::ApplyShield(AActor* Target)
{
	if (!Target || ShieldAmount <= 0.0f)
	{
		return;
	}

	if (UAbilitySystemComponent* ASC = ResolveHolyShieldASC(Target))
	{
		if (UDBABattleAttributeSet* AttrSet = const_cast<UDBABattleAttributeSet*>(ASC->GetSet<UDBABattleAttributeSet>()))
		{
			AppliedShieldAmount = ShieldAmount;
			const float ExistingMaxShield = AttrSet->GetMaxShield();
			const float ExistingCurrentShield = AttrSet->GetCurrentShield();
			AttrSet->SetMaxShield(FMath::Max(ExistingMaxShield, ExistingCurrentShield + AppliedShieldAmount));
			AttrSet->SetCurrentShield(FMath::Clamp(ExistingCurrentShield + AppliedShieldAmount, 0.0f, AttrSet->GetMaxShield()));
		}
	}
}

void ADBAHolyShieldSpell::ReleaseShield()
{
	if (ActiveBarrierVFX)
	{
		ActiveBarrierVFX->Deactivate();
		ActiveBarrierVFX = nullptr;
	}

	AActor* Target = ShieldTarget.Get();
	if (!Target || AppliedShieldAmount <= 0.0f)
	{
		Destroy();
		return;
	}

	if (UAbilitySystemComponent* ASC = ResolveHolyShieldASC(Target))
	{
		if (UDBABattleAttributeSet* AttrSet = const_cast<UDBABattleAttributeSet*>(ASC->GetSet<UDBABattleAttributeSet>()))
		{
			const float NewShield = FMath::Max(AttrSet->GetCurrentShield() - AppliedShieldAmount, 0.0f);
			AttrSet->SetCurrentShield(NewShield);
			AttrSet->SetMaxShield(FMath::Max(AttrSet->GetMaxShield() - AppliedShieldAmount, NewShield));
		}
	}

	const FVector Location = ShieldCenterLocation(Target);
	MulticastPlayShieldEnd(Location);
	if (GetNetMode() == NM_Standalone)
	{
		MulticastPlayShieldEnd_Implementation(Location);
	}
	Destroy();
}

void ADBAHolyShieldSpell::MulticastPlayShieldStart_Implementation(
	AActor* TargetActor,
	FVector_NetQuantize SourceLocation,
	FVector_NetQuantize FlightTargetLocation,
	FVector_NetQuantize ImpactLocation)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	const FVector Source(SourceLocation);
	const FVector FlightTarget(FlightTargetLocation);
	const FVector Center(ImpactLocation);
	SpawnVFX(CastVFXAsset, Source, FRotator::ZeroRotator, FVector(1.0f));
	SpawnTravelVFX(FlightVFXAsset, Source, FlightTarget, 0.72f);
	if (TargetActor)
	{
		ActiveBarrierVFX = SpawnAttachedVFX(BarrierVFXAsset, TargetActor, FVector(0.0f, 0.0f, 50.0f), FRotator::ZeroRotator, FVector(1.24f), false);
	}
	else
	{
		ActiveBarrierVFX = SpawnVFX(BarrierVFXAsset, Center - FVector(0.0f, 0.0f, 24.0f), FRotator::ZeroRotator, FVector(1.24f), false);
	}
	SpawnVFX(ImpactVFXAsset, Center, FRotator::ZeroRotator, FVector(0.74f));
	PlaySFX(CastSFXAsset, Source, 0.82f);
	PlaySFX(SustainSFXAsset, (Source + FlightTarget) * 0.5f, 0.58f);
	PlaySFX(ImpactSFXAsset, Center, 0.70f);
}

void ADBAHolyShieldSpell::MulticastPlayShieldEnd_Implementation(FVector_NetQuantize Location)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	const FVector Center(Location);
	if (ActiveBarrierVFX)
	{
		ActiveBarrierVFX->Deactivate();
		ActiveBarrierVFX = nullptr;
	}
	SpawnVFX(ImpactVFXAsset, Center, FRotator::ZeroRotator, FVector(0.56f));
}

UNiagaraComponent* ADBAHolyShieldSpell::SpawnVFX(const TSoftObjectPtr<UNiagaraSystem>& Asset, const FVector& Location, const FRotator& Rotation, const FVector& Scale, bool bAutoDestroy) const
{
	if (Asset.IsNull() || GetNetMode() == NM_DedicatedServer)
	{
		return nullptr;
	}

	auto SpawnLoaded = [this, Location, Rotation, Scale, bAutoDestroy](UNiagaraSystem* VFX) -> UNiagaraComponent*
	{
		UNiagaraComponent* SpawnedVFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			VFX,
			Location,
			Rotation,
			Scale,
			true,
			bAutoDestroy,
			ENCPoolMethod::AutoRelease,
			true);
		UDBANiagaraSkillParameterLibrary::ApplySkillParameters(
			SpawnedVFX,
			NiagaraParameters,
			ShieldAmount,
			Location,
			Rotation.Vector(),
			0.0f,
			NiagaraParameters.EffectRadius);
		return SpawnedVFX;
	};

	if (UNiagaraSystem* LoadedVFX = Asset.Get())
	{
		return SpawnLoaded(LoadedVFX);
	}

	UWorld* World = GetWorld();
	const FDBANiagaraSkillParameters CapturedParameters = NiagaraParameters;
	const float CapturedShieldAmount = ShieldAmount;
	DBAAsyncAssetLoader::RequestAsyncAsset<UNiagaraSystem>(World, Asset, [World, Location, Rotation, Scale, bAutoDestroy, CapturedParameters, CapturedShieldAmount](UNiagaraSystem* LoadedVFX)
	{
		if (!IsUsableHolyShieldWorld(World) || !LoadedVFX)
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
			bAutoDestroy,
			ENCPoolMethod::AutoRelease,
			true);
		UDBANiagaraSkillParameterLibrary::ApplySkillParameters(
			SpawnedVFX,
			CapturedParameters,
			CapturedShieldAmount,
			Location,
			Rotation.Vector(),
			0.0f,
			CapturedParameters.EffectRadius);
	});

	return nullptr;
}

UNiagaraComponent* ADBAHolyShieldSpell::SpawnAttachedVFX(const TSoftObjectPtr<UNiagaraSystem>& Asset, AActor* TargetActor, const FVector& RelativeOffset, const FRotator& Rotation, const FVector& Scale, bool bAutoDestroy) const
{
	if (Asset.IsNull() || !TargetActor || !TargetActor->GetRootComponent() || GetNetMode() == NM_DedicatedServer)
	{
		return nullptr;
	}

	auto SpawnLoaded = [this, TargetActor, RelativeOffset, Rotation, Scale, bAutoDestroy](UNiagaraSystem* VFX) -> UNiagaraComponent*
	{
		if (!TargetActor || !TargetActor->GetRootComponent())
		{
			return nullptr;
		}
		UNiagaraComponent* SpawnedVFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
			VFX,
			TargetActor->GetRootComponent(),
			NAME_None,
			RelativeOffset,
			Rotation,
			Scale,
			EAttachLocation::KeepRelativeOffset,
			bAutoDestroy,
			ENCPoolMethod::AutoRelease,
			true,
			true);
		UDBANiagaraSkillParameterLibrary::ApplySkillParameters(
			SpawnedVFX,
			NiagaraParameters,
			ShieldAmount,
			TargetActor->GetActorLocation(),
			TargetActor->GetActorForwardVector(),
			0.0f,
			NiagaraParameters.EffectRadius);
		return SpawnedVFX;
	};

	if (UNiagaraSystem* LoadedVFX = Asset.Get())
	{
		return SpawnLoaded(LoadedVFX);
	}

	TWeakObjectPtr<AActor> WeakTarget(TargetActor);
	const FDBANiagaraSkillParameters CapturedParameters = NiagaraParameters;
	const float CapturedShieldAmount = ShieldAmount;
	DBAAsyncAssetLoader::RequestAsyncAsset<UNiagaraSystem>(TargetActor, Asset, [WeakTarget, RelativeOffset, Rotation, Scale, bAutoDestroy, CapturedParameters, CapturedShieldAmount](UNiagaraSystem* LoadedVFX)
	{
		AActor* StrongTarget = WeakTarget.Get();
		if (!StrongTarget || !StrongTarget->GetRootComponent() || StrongTarget->GetNetMode() == NM_DedicatedServer || !LoadedVFX)
		{
			return;
		}

		UNiagaraComponent* SpawnedVFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
			LoadedVFX,
			StrongTarget->GetRootComponent(),
			NAME_None,
			RelativeOffset,
			Rotation,
			Scale,
			EAttachLocation::KeepRelativeOffset,
			bAutoDestroy,
			ENCPoolMethod::AutoRelease,
			true,
			true);
		UDBANiagaraSkillParameterLibrary::ApplySkillParameters(
			SpawnedVFX,
			CapturedParameters,
			CapturedShieldAmount,
			StrongTarget->GetActorLocation(),
			StrongTarget->GetActorForwardVector(),
			0.0f,
			CapturedParameters.EffectRadius);
	});

	return nullptr;
}

void ADBAHolyShieldSpell::SpawnTravelVFX(const TSoftObjectPtr<UNiagaraSystem>& Asset, const FVector& SourceLocation, const FVector& TargetLocation, float WidthScale) const
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
	const FRotator Rotation = ShieldTravelRotation(SourceLocation, HorizontalTarget);
	const FVector Scale(FMath::Max(Distance / 360.0f, 0.34f), WidthScale, WidthScale);
	SpawnVFX(Asset, MidPoint, Rotation, Scale);
}

void ADBAHolyShieldSpell::PlaySFX(const TSoftObjectPtr<USoundBase>& Asset, const FVector& Location, float Volume) const
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
			if (!IsUsableHolyShieldWorld(World) || !LoadedSFX)
			{
				return;
			}
			UGameplayStatics::PlaySoundAtLocation(World, LoadedSFX, Location, Volume);
		});
	}
}

void ADBAHolyShieldSpell::PreloadPresentationAssets()
{
	TArray<FSoftObjectPath> Paths;
	DBAAsyncAssetLoader::AddPreloadPath(CastVFXAsset, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(BarrierVFXAsset, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(FlightVFXAsset, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(ImpactVFXAsset, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(CastSFXAsset, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(SustainSFXAsset, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(ImpactSFXAsset, Paths);
	DBAAsyncAssetLoader::RequestAsyncPreload(this, Paths);
}
