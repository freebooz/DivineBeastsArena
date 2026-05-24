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
	ImpactVFXAsset = TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(TEXT("/Game/ProjectileHitVFX/NS/NS_HolyEnergy.NS_HolyEnergy")));
	CastSFXAsset = TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Game/DBA/Audio/SFX/Downloaded/ClassMagic/SFX_PriestShield_PreCast.SFX_PriestShield_PreCast")));
	SustainSFXAsset = TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Game/DBA/Audio/SFX/Downloaded/ClassMagic/SFX_PriestShield_Flight.SFX_PriestShield_Flight")));
	ImpactSFXAsset = TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Game/DBA/Audio/SFX/Downloaded/ClassMagic/SFX_PriestShield_Impact.SFX_PriestShield_Impact")));
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
	PreloadPresentationAssets();
	MulticastPlayShieldStart(ShieldTarget, Location);
	if (GetNetMode() == NM_Standalone)
	{
		MulticastPlayShieldStart_Implementation(ShieldTarget, Location);
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

void ADBAHolyShieldSpell::MulticastPlayShieldStart_Implementation(AActor* TargetActor, FVector_NetQuantize Location)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	const FVector Center(Location);
	SpawnVFX(CastVFXAsset, Center, FRotator::ZeroRotator, FVector(1.0f));
	if (TargetActor)
	{
		ActiveBarrierVFX = SpawnAttachedVFX(BarrierVFXAsset, TargetActor, FVector(0.0f, 0.0f, 50.0f), FRotator::ZeroRotator, FVector(1.24f), false);
	}
	else
	{
		ActiveBarrierVFX = SpawnVFX(BarrierVFXAsset, Center - FVector(0.0f, 0.0f, 24.0f), FRotator::ZeroRotator, FVector(1.24f), false);
	}
	SpawnVFX(ImpactVFXAsset, Center, FRotator::ZeroRotator, FVector(0.74f));
	PlaySFX(CastSFXAsset, Center, 0.82f);
	PlaySFX(SustainSFXAsset, Center, 0.58f);
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
		return UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			VFX,
			Location,
			Rotation,
			Scale,
			true,
			bAutoDestroy,
			ENCPoolMethod::AutoRelease,
			true);
	};

	if (UNiagaraSystem* LoadedVFX = Asset.Get())
	{
		return SpawnLoaded(LoadedVFX);
	}

	TArray<FSoftObjectPath> Paths;
	DBAAsyncAssetLoader::AddPreloadPath(Asset, Paths);
	DBAAsyncAssetLoader::RequestAsyncPreload(const_cast<ADBAHolyShieldSpell*>(this), Paths);

	return nullptr;
}

UNiagaraComponent* ADBAHolyShieldSpell::SpawnAttachedVFX(const TSoftObjectPtr<UNiagaraSystem>& Asset, AActor* TargetActor, const FVector& RelativeOffset, const FRotator& Rotation, const FVector& Scale, bool bAutoDestroy) const
{
	if (Asset.IsNull() || !TargetActor || !TargetActor->GetRootComponent() || GetNetMode() == NM_DedicatedServer)
	{
		return nullptr;
	}

	auto SpawnLoaded = [TargetActor, RelativeOffset, Rotation, Scale, bAutoDestroy](UNiagaraSystem* VFX) -> UNiagaraComponent*
	{
		if (!TargetActor || !TargetActor->GetRootComponent())
		{
			return nullptr;
		}
		return UNiagaraFunctionLibrary::SpawnSystemAttached(
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
	};

	if (UNiagaraSystem* LoadedVFX = Asset.Get())
	{
		return SpawnLoaded(LoadedVFX);
	}

	TArray<FSoftObjectPath> Paths;
	DBAAsyncAssetLoader::AddPreloadPath(Asset, Paths);
	DBAAsyncAssetLoader::RequestAsyncPreload(const_cast<ADBAHolyShieldSpell*>(this), Paths);

	return nullptr;
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
		TArray<FSoftObjectPath> Paths;
		DBAAsyncAssetLoader::AddPreloadPath(Asset, Paths);
		DBAAsyncAssetLoader::RequestAsyncPreload(const_cast<ADBAHolyShieldSpell*>(this), Paths);
	}
}

void ADBAHolyShieldSpell::PreloadPresentationAssets()
{
	TArray<FSoftObjectPath> Paths;
	DBAAsyncAssetLoader::AddPreloadPath(CastVFXAsset, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(BarrierVFXAsset, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(ImpactVFXAsset, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(CastSFXAsset, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(SustainSFXAsset, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(ImpactSFXAsset, Paths);
	DBAAsyncAssetLoader::RequestAsyncPreload(this, Paths);
}
