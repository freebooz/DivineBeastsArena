// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Combat/DBASkillProjectileBase.h"

#include "AbilitySystemComponent.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameDBA/Character/DBAZodiacCharacterBase.h"
#include "GameDBA/Combat/DBADamageCalculator.h"
#include "Engine/World.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/GAS/Attributes/DBABattleAttributeSet.h"
#include "GameDBA/GAS/DBAAbilitySystemComponent.h"
#include "GameDBA/Utilities/DBAAsyncAssetLoader.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"

namespace
{
	FString SoftObjectPathString(const FSoftObjectPath& Path)
	{
		return Path.IsValid() ? Path.ToString() : FString();
	}

	UAbilitySystemComponent* ResolveProjectileAbilitySystemComponent(AActor* Actor)
	{
		if (!Actor)
		{
			return nullptr;
		}

		if (UAbilitySystemComponent* ASC = Actor->FindComponentByClass<UAbilitySystemComponent>())
		{
			return ASC;
		}

		AActor* Owner = Actor->GetOwner();
		return Owner ? Owner->FindComponentByClass<UAbilitySystemComponent>() : nullptr;
	}

	EDBAElement ToProjectileCommonElement(EDBAElementType ElementType)
	{
		switch (ElementType)
		{
		case EDBAElementType::Fire: return EDBAElement::Fire;
		case EDBAElementType::Water: return EDBAElement::Water;
		case EDBAElementType::Wood: return EDBAElement::Wood;
		case EDBAElementType::Metal: return EDBAElement::Gold;
		case EDBAElementType::Earth: return EDBAElement::Earth;
		default: return EDBAElement::None;
		}
	}

	EDBAElement ResolveElementFromActor(AActor* Actor, EDBAElement FallbackElement)
	{
		if (const ADBAZodiacCharacterBase* ZodiacCharacter = Cast<ADBAZodiacCharacterBase>(Actor))
		{
			const EDBAElement ResolvedElement = ToProjectileCommonElement(ZodiacCharacter->GetElementType());
			return ResolvedElement == EDBAElement::None ? FallbackElement : ResolvedElement;
		}
		return FallbackElement;
	}

	float ResolveDefense(AActor* Actor)
	{
		if (UAbilitySystemComponent* ASC = ResolveProjectileAbilitySystemComponent(Actor))
		{
			if (const UDBABattleAttributeSet* BattleAttributes = ASC->GetSet<UDBABattleAttributeSet>())
			{
				return BattleAttributes->GetDefense();
			}
		}
		return 0.0f;
	}

	int32 ResolveResonanceLevel(AActor* Actor)
	{
		if (const ADBAZodiacCharacterBase* ZodiacCharacter = Cast<ADBAZodiacCharacterBase>(Actor))
		{
			return ZodiacCharacter->GetResonanceLevel();
		}
		if (const UDBAAbilitySystemComponent* ASC = Cast<UDBAAbilitySystemComponent>(ResolveProjectileAbilitySystemComponent(Actor)))
		{
			return ASC->GetResonanceLevel();
		}
		return 0;
	}

	int32 ResolveChainLevel(AActor* Actor)
	{
		if (const ADBAZodiacCharacterBase* ZodiacCharacter = Cast<ADBAZodiacCharacterBase>(Actor))
		{
			return ZodiacCharacter->GetChainLevel();
		}
		if (const UDBAAbilitySystemComponent* ASC = Cast<UDBAAbilitySystemComponent>(ResolveProjectileAbilitySystemComponent(Actor)))
		{
			return ASC->GetChainLevel();
		}
		return 0;
	}

	float ResolveCriticalRate(AActor* Actor, float FallbackCriticalRate)
	{
		if (UAbilitySystemComponent* ASC = ResolveProjectileAbilitySystemComponent(Actor))
		{
			if (const UDBABattleAttributeSet* BattleAttributes = ASC->GetSet<UDBABattleAttributeSet>())
			{
				return BattleAttributes->GetCriticalRate();
			}
		}
		return FMath::Max(FallbackCriticalRate, 0.0f);
	}

	FGameplayTag ResolveCueTag(const FGameplayTag& ConfiguredTag, const TCHAR* FallbackTagName)
	{
		if (ConfiguredTag.IsValid())
		{
			return ConfiguredTag;
		}
		return FallbackTagName ? FGameplayTag::RequestGameplayTag(FName(FallbackTagName), false) : FGameplayTag();
	}

	void ExecuteProjectileGameplayCue(
		AActor* ProjectileOwner,
		AActor* CueTarget,
		const FGameplayTag& CueTag,
		const FVector& Location,
		float RawMagnitude,
		bool bIsCritical)
	{
		if (!CueTag.IsValid())
		{
			return;
		}

		UAbilitySystemComponent* CueASC = ResolveProjectileAbilitySystemComponent(CueTarget);
		if (!CueASC)
		{
			CueASC = ResolveProjectileAbilitySystemComponent(ProjectileOwner);
		}
		if (!CueASC)
		{
			return;
		}

		FGameplayCueParameters CueParams;
		CueParams.Instigator = ProjectileOwner;
		CueParams.EffectCauser = ProjectileOwner;
		CueParams.SourceObject = ProjectileOwner;
		CueParams.Location = Location;
		CueParams.RawMagnitude = RawMagnitude;
		CueParams.NormalizedMagnitude = bIsCritical ? 1.0f : 0.0f;
		CueASC->ExecuteGameplayCue(CueTag, CueParams);
		if (UDBAAbilitySystemComponent* DBAASC = Cast<UDBAAbilitySystemComponent>(CueASC))
		{
			DBAASC->OnSkillCueExecuted.Broadcast(CueTag.GetTagName(), CueTarget);
		}
	}
}

ADBASkillProjectileBase::ADBASkillProjectileBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);
	InitialLifeSpan = 5.0f;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionSphere->InitSphereRadius(Radius);
	CollisionSphere->SetCollisionProfileName(TEXT("Projectile"));
	CollisionSphere->SetGenerateOverlapEvents(true);
	CollisionSphere->OnComponentHit.AddDynamic(this, &ADBASkillProjectileBase::HandleProjectileHit);
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ADBASkillProjectileBase::HandleProjectileOverlap);
	RootComponent = CollisionSphere;

	UStaticMeshComponent* MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetHiddenInGame(true);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = RootComponent;
	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed * 1.5f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bInitialVelocityInLocalSpace = false;
	ProjectileMovement->ProjectileGravityScale = 0.0f;

	ProjectileVFX = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ProjectileVFX"));
	ProjectileVFX->SetupAttachment(RootComponent);
	ProjectileVFX->bAutoActivate = false;
	ProjectileVFX->SetVisibility(false);

	ProjectileNiagaraVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ProjectileNiagaraVFX"));
	ProjectileNiagaraVFX->SetupAttachment(RootComponent);
	ProjectileNiagaraVFX->bAutoActivate = false;
	ProjectileNiagaraVFX->SetVisibility(false);

	ProjectileLoopAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("ProjectileLoopAudio"));
	ProjectileLoopAudio->SetupAttachment(RootComponent);
	ProjectileLoopAudio->bAutoActivate = false;
	ProjectileLoopAudio->bAllowSpatialization = true;
}

void ADBASkillProjectileBase::InitializeProjectile(
	FName InSkillId,
	AActor* InOwner,
	AActor* InTarget,
	float InDamage,
	float InSpeed,
	float InRadius)
{
	SkillId = InSkillId;
	ProjectileOwner = InOwner;
	TargetActor = InTarget;
	SetProjectileProperties(InSpeed, InRadius, InDamage);

	if (CollisionSphere && InOwner)
	{
		CollisionSphere->IgnoreActorWhenMoving(InOwner, true);
	}

	const FString ProjectileVFXPath = SoftObjectPathString(ProjectileVFXAsset.ToSoftObjectPath());
	const FString ProjectileNiagaraVFXPath = SoftObjectPathString(ProjectileNiagaraVFXAsset.ToSoftObjectPath());
	const FString FlySFXPath = SoftObjectPathString(FlySFXAsset.ToSoftObjectPath());
	PreloadPresentationAssets();
	ApplyProjectileVisualsLocal(ProjectileVFXPath, ProjectileNiagaraVFXPath, FlySFXPath);
	if (HasAuthority() && GetNetMode() != NM_Standalone)
	{
		MulticastApplyProjectileVisuals(ProjectileVFXPath, ProjectileNiagaraVFXPath, FlySFXPath);
	}

	ExecuteProjectileGameplayCue(ProjectileOwner, ProjectileOwner, ResolveCueTag(ProjectileCueTag, TEXT("GameplayCue.DBA.Skill.Projectile")), GetActorLocation(), Damage, false);

	if (InTarget)
	{
		LaunchProjectile(InTarget->GetActorLocation() - GetActorLocation());
	}
}

void ADBASkillProjectileBase::MulticastApplyProjectileVisuals_Implementation(
	const FString& ProjectileVFXPath,
	const FString& ProjectileNiagaraVFXPath,
	const FString& FlySFXPath)
{
	if (HasAuthority())
	{
		return;
	}

	ApplyProjectileVisualsLocal(ProjectileVFXPath, ProjectileNiagaraVFXPath, FlySFXPath);
}

void ADBASkillProjectileBase::ApplyProjectileVisualsLocal(
	const FString& ProjectileVFXPath,
	const FString& ProjectileNiagaraVFXPath,
	const FString& FlySFXPath)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (!ProjectileVFXPath.IsEmpty() && ProjectileVFX)
	{
		TSoftObjectPtr<UParticleSystem> VFXAsset{FSoftObjectPath(ProjectileVFXPath)};
		if (UParticleSystem* VFX = VFXAsset.Get())
		{
			ProjectileVFX->SetTemplate(VFX);
			ProjectileVFX->SetVisibility(true);
			ProjectileVFX->Activate(true);
		}
		else
		{
			DBAAsyncAssetLoader::RequestAsyncAsset<UParticleSystem>(this, VFXAsset, [this](UParticleSystem* LoadedVFX)
			{
				if (!ProjectileVFX || bProjectileHitProcessed)
				{
					return;
				}
				ProjectileVFX->SetTemplate(LoadedVFX);
				ProjectileVFX->SetVisibility(true);
				ProjectileVFX->Activate(true);
			});
		}
	}

	if (!ProjectileNiagaraVFXPath.IsEmpty() && ProjectileNiagaraVFX)
	{
		TSoftObjectPtr<UNiagaraSystem> VFXAsset{FSoftObjectPath(ProjectileNiagaraVFXPath)};
		if (UNiagaraSystem* VFX = VFXAsset.Get())
		{
			ProjectileNiagaraVFX->SetAsset(VFX);
			ProjectileNiagaraVFX->SetVisibility(true);
			ProjectileNiagaraVFX->Activate(true);
		}
		else
		{
			DBAAsyncAssetLoader::RequestAsyncAsset<UNiagaraSystem>(this, VFXAsset, [this](UNiagaraSystem* LoadedVFX)
			{
				if (!ProjectileNiagaraVFX || bProjectileHitProcessed)
				{
					return;
				}
				ProjectileNiagaraVFX->SetAsset(LoadedVFX);
				ProjectileNiagaraVFX->SetVisibility(true);
				ProjectileNiagaraVFX->Activate(true);
			});
		}
	}

	if (!FlySFXPath.IsEmpty() && ProjectileLoopAudio)
	{
		TSoftObjectPtr<USoundBase> SFXAsset{FSoftObjectPath(FlySFXPath)};
		if (USoundBase* FlySFX = SFXAsset.Get())
		{
			ProjectileLoopAudio->SetSound(FlySFX);
			ProjectileLoopAudio->Play();
		}
		else
		{
			DBAAsyncAssetLoader::RequestAsyncAsset<USoundBase>(this, SFXAsset, [this](USoundBase* LoadedSFX)
			{
				if (!ProjectileLoopAudio || bProjectileHitProcessed)
				{
					return;
				}
				ProjectileLoopAudio->SetSound(LoadedSFX);
				ProjectileLoopAudio->Play();
			});
		}
	}
}

void ADBASkillProjectileBase::SetProjectileProperties(float InSpeed, float InRadius, float InDamage)
{
	Speed = InSpeed;
	Radius = InRadius;
	Damage = InDamage;

	if (ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = Speed;
		ProjectileMovement->MaxSpeed = Speed * 1.5f;
	}

	if (CollisionSphere)
	{
		CollisionSphere->SetSphereRadius(Radius);
	}
}

void ADBASkillProjectileBase::LaunchProjectile(const FVector& Direction)
{
	if (!ProjectileMovement)
	{
		return;
	}

	const FVector SafeDirection = Direction.GetSafeNormal();
	if (SafeDirection.IsNearlyZero())
	{
		return;
	}

	ProjectileMovement->Velocity = SafeDirection * Speed;
	SetActorRotation(SafeDirection.Rotation());
}

void ADBASkillProjectileBase::OnProjectileHit(AActor* HitActor, FVector HitLocation)
{
	if (bProjectileHitProcessed)
	{
		return;
	}
	bProjectileHitProcessed = true;

	SetActorEnableCollision(false);
	if (CollisionSphere)
	{
		CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CollisionSphere->SetGenerateOverlapEvents(false);
	}
	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->Deactivate();
	}
	if (ProjectileVFX)
	{
		ProjectileVFX->DeactivateSystem();
	}
	if (ProjectileNiagaraVFX)
	{
		ProjectileNiagaraVFX->Deactivate();
	}

	const FString ImpactVFXPath = SoftObjectPathString(ImpactVFXAsset.ToSoftObjectPath());
	const FString ImpactNiagaraVFXPath = SoftObjectPathString(ImpactNiagaraVFXAsset.ToSoftObjectPath());
	const FString ImpactSFXPath = SoftObjectPathString(ImpactSFXAsset.ToSoftObjectPath());
	PlayImpactFeedbackLocal(ImpactVFXPath, ImpactNiagaraVFXPath, ImpactSFXPath, HitLocation, GetActorRotation());
	if (HasAuthority() && GetNetMode() != NM_Standalone)
	{
		MulticastPlayImpactFeedback(ImpactVFXPath, ImpactNiagaraVFXPath, ImpactSFXPath, HitLocation, GetActorRotation());
	}

	if (HitActor && HitActor != ProjectileOwner && Damage > 0.0f)
	{
		bool bIsCritical = false;
		const EDBAElement EffectiveAttackElement = DamageElement == EDBAElement::None
			? ResolveElementFromActor(ProjectileOwner, EDBAElement::None)
			: DamageElement;
		const int32 EffectiveResonanceLevel = ResonanceLevelOverride >= 0 ? ResonanceLevelOverride : ResolveResonanceLevel(ProjectileOwner);
		const int32 EffectiveChainLevel = ChainLevelOverride >= 0 ? ChainLevelOverride : ResolveChainLevel(ProjectileOwner);
		const float EffectiveCriticalRate = CriticalRateOverride >= 0.0f ? CriticalRateOverride : ResolveCriticalRate(ProjectileOwner, 0.0f);
		const float FinalDamage = UDBADamageCalculator::CalculateFinalDamage(
			Damage,
			EffectiveAttackElement,
			ResolveElementFromActor(HitActor, EDBAElement::None),
			EffectiveResonanceLevel,
			EffectiveChainLevel,
			ResolveDefense(HitActor),
			EffectiveCriticalRate,
			FMath::Max(CriticalMultiplier, 1.0f),
			bIsCritical);

		UDBADamageCalculator::ApplyDamageToTargetWithCue(
			ProjectileOwner,
			HitActor,
			FinalDamage,
			EffectiveAttackElement,
			bIsCritical,
			ResolveCueTag(ImpactCueTag, TEXT("GameplayCue.DBA.Skill.Impact")),
			HitLocation);
	}

	if (ProjectileLoopAudio)
	{
		ProjectileLoopAudio->FadeOut(0.08f, 0.0f);
	}

	BP_OnProjectileHit(HitActor, HitLocation);
	Destroy();
}

void ADBASkillProjectileBase::MulticastPlayImpactFeedback_Implementation(
	const FString& ImpactVFXPath,
	const FString& ImpactNiagaraVFXPath,
	const FString& ImpactSFXPath,
	FVector_NetQuantize HitLocation,
	FRotator HitRotation)
{
	if (HasAuthority())
	{
		return;
	}

	PlayImpactFeedbackLocal(ImpactVFXPath, ImpactNiagaraVFXPath, ImpactSFXPath, HitLocation, HitRotation);
}

void ADBASkillProjectileBase::PlayImpactFeedbackLocal(
	const FString& ImpactVFXPath,
	const FString& ImpactNiagaraVFXPath,
	const FString& ImpactSFXPath,
	const FVector& HitLocation,
	const FRotator& HitRotation)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (!ImpactNiagaraVFXPath.IsEmpty())
	{
		TSoftObjectPtr<UNiagaraSystem> VFXAsset{FSoftObjectPath(ImpactNiagaraVFXPath)};
		if (UNiagaraSystem* VFX = VFXAsset.Get())
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				VFX,
				HitLocation,
				HitRotation,
				FVector(1.0f),
				true,
				true,
				ENCPoolMethod::AutoRelease,
				true);
		}
		else
		{
			TArray<FSoftObjectPath> Paths;
			DBAAsyncAssetLoader::AddPreloadPath(VFXAsset, Paths);
			DBAAsyncAssetLoader::RequestAsyncPreload(this, Paths);
		}
	}

	if (!ImpactVFXPath.IsEmpty())
	{
		TSoftObjectPtr<UParticleSystem> VFXAsset{FSoftObjectPath(ImpactVFXPath)};
		if (UParticleSystem* VFX = VFXAsset.Get())
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, HitLocation, FRotator::ZeroRotator, true);
		}
		else
		{
			TArray<FSoftObjectPath> Paths;
			DBAAsyncAssetLoader::AddPreloadPath(VFXAsset, Paths);
			DBAAsyncAssetLoader::RequestAsyncPreload(this, Paths);
		}
	}

	if (!ImpactSFXPath.IsEmpty())
	{
		TSoftObjectPtr<USoundBase> SFXAsset{FSoftObjectPath(ImpactSFXPath)};
		if (USoundBase* SFX = SFXAsset.Get())
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, HitLocation);
		}
		else
		{
			TArray<FSoftObjectPath> Paths;
			DBAAsyncAssetLoader::AddPreloadPath(SFXAsset, Paths);
			DBAAsyncAssetLoader::RequestAsyncPreload(this, Paths);
		}
	}
}

void ADBASkillProjectileBase::SetCollisionChannel(ECollisionChannel Channel)
{
	if (CollisionSphere)
	{
		CollisionSphere->SetCollisionObjectType(Channel);
	}
}

void ADBASkillProjectileBase::PreloadPresentationAssets()
{
	TArray<FSoftObjectPath> Paths;
	DBAAsyncAssetLoader::AddPreloadPath(ProjectileVFXAsset, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(ImpactVFXAsset, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(ProjectileNiagaraVFXAsset, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(ImpactNiagaraVFXAsset, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(FlySFXAsset, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(ImpactSFXAsset, Paths);
	DBAAsyncAssetLoader::RequestAsyncPreload(this, Paths);
}

void ADBASkillProjectileBase::HandleProjectileHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!bProjectileHitProcessed && OtherActor && OtherActor != this && OtherActor != ProjectileOwner)
	{
		const FVector ImpactPoint = Hit.ImpactPoint.IsNearlyZero() ? GetActorLocation() : FVector(Hit.ImpactPoint);
		OnProjectileHit(OtherActor, ImpactPoint);
	}
}

void ADBASkillProjectileBase::HandleProjectileOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!bProjectileHitProcessed && OtherActor && OtherActor != this && OtherActor != ProjectileOwner)
	{
		const FVector ImpactPoint = SweepResult.ImpactPoint.IsNearlyZero() ? GetActorLocation() : FVector(SweepResult.ImpactPoint);
		OnProjectileHit(OtherActor, ImpactPoint);
	}
}
