// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// 泛化技能VFX组件实现

#include "GameDBA/VFX/Components/Skill/DBAZodiacSkillVFXComponent_Generic.h"
#include "AbilitySystemComponent.h"
#include "GameDBA/Character/DBAZodiacCharacterBase.h"
#include "GameDBA/Combat/DBADamageCalculator.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/GAS/Attributes/DBABattleAttributeSet.h"
#include "GameDBA/GAS/DBAAbilitySystemComponent.h"
#include "GameDBA/Utilities/DBAAsyncAssetLoader.h"
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"

namespace
{
	UAbilitySystemComponent* ResolveAbilitySystemComponent(AActor* Actor)
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

	EDBAElement ToCommonElement(EDBAElementType ElementType)
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
			const EDBAElement ResolvedElement = ToCommonElement(ZodiacCharacter->GetElementType());
			return ResolvedElement == EDBAElement::None ? FallbackElement : ResolvedElement;
		}

		return FallbackElement;
	}

	float ResolveDefense(AActor* Actor)
	{
		if (UAbilitySystemComponent* ASC = ResolveAbilitySystemComponent(Actor))
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
		if (const UDBAAbilitySystemComponent* ASC = Cast<UDBAAbilitySystemComponent>(ResolveAbilitySystemComponent(Actor)))
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
		if (const UDBAAbilitySystemComponent* ASC = Cast<UDBAAbilitySystemComponent>(ResolveAbilitySystemComponent(Actor)))
		{
			return ASC->GetChainLevel();
		}
		return 0;
	}

	float ResolveCriticalRate(AActor* Actor, float FallbackCriticalRate)
	{
		if (UAbilitySystemComponent* ASC = ResolveAbilitySystemComponent(Actor))
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
}

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
		PreloadCachedVFXDataResources();
		UE_LOG(LogDBACombat, Log, TEXT("[DBAZodiacSkillVFXComponent_Generic] 加载VFX配置: %s"), *RowName.ToString());
	}
	else
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[DBAZodiacSkillVFXComponent_Generic] 未找到VFX配置: %s"), *RowName.ToString());
	}
}

const FDBAVFXDataRow& UDBAZodiacSkillVFXComponent_Generic::GetVFXData() const
{
	return CachedVFXData;
}

void UDBAZodiacSkillVFXComponent_Generic::PreloadCachedVFXDataResources()
{
	TArray<FSoftObjectPath> Paths;
	DBAAsyncAssetLoader::AddPreloadPath(CachedVFXData.CastingVFX, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(CachedVFXData.ImpactVFX, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(CachedVFXData.ProjectileVFX, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(CachedVFXData.AOEVFX, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(CachedVFXData.ChannelVFX, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(CachedVFXData.BuffVFX, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(CachedVFXData.DebuffVFX, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(CachedVFXData.CastingSFX, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(CachedVFXData.ProjectileSFX, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(CachedVFXData.ImpactSFX, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(CachedVFXData.CastingMontage, Paths);
	DBAAsyncAssetLoader::AddPreloadPath(CachedVFXData.ImpactMontage, Paths);
	DBAAsyncAssetLoader::RequestAsyncPreload(this, Paths);
}

void UDBAZodiacSkillVFXComponent_Generic::PlayCastingVFX(AActor* Target)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	if (UParticleSystem* VFX = CachedVFXData.CastingVFX.Get())
	{
		FVector Location = Target ? Target->GetActorLocation() : OwnerActor->GetActorLocation();
		FRotator Rotation = OwnerActor->GetActorRotation();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, Location, Rotation, true);
	}

	if (USoundBase* SFX = CachedVFXData.CastingSFX.Get())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, OwnerActor->GetActorLocation());
	}

	if (UAnimMontage* Montage = CachedVFXData.CastingMontage.Get())
	{
		if (USkeletalMeshComponent* Mesh = OwnerActor->FindComponentByClass<USkeletalMeshComponent>())
		{
			if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
			{
				AnimInstance->Montage_Play(Montage);
			}
		}
	}

	ExecuteSkillGameplayCue(ResolveCueTag(CastingCueTag, TEXT("GameplayCue.DBA.Skill.Cast")), Target ? Target : OwnerActor, OwnerActor->GetActorLocation(), 0.0f, false);
}

void UDBAZodiacSkillVFXComponent_Generic::PlayImpactVFX(AActor* HitTarget)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	const FVector ImpactLocation = HitTarget ? HitTarget->GetActorLocation() : OwnerActor->GetActorLocation();
	if (UParticleSystem* VFX = CachedVFXData.ImpactVFX.Get())
	{
		FRotator Rotation = FRotator::ZeroRotator;
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, ImpactLocation, Rotation, true);
	}

	if (USoundBase* SFX = CachedVFXData.ImpactSFX.Get())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, ImpactLocation);
	}

	if (UAnimMontage* Montage = CachedVFXData.ImpactMontage.Get())
	{
		if (HitTarget)
		{
			if (USkeletalMeshComponent* Mesh = HitTarget->FindComponentByClass<USkeletalMeshComponent>())
			{
				if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
				{
					AnimInstance->Montage_Play(Montage);
				}
			}
		}
	}

	if (bApplyDamageOnImpact && HitTarget && HitTarget != OwnerActor)
	{
		float FinalDamage = 0.0f;
		bool bIsCritical = false;
		ApplySkillDamage(HitTarget, ImpactLocation, FinalDamage, bIsCritical);
	}
	else
	{
		ExecuteSkillGameplayCue(ResolveCueTag(ImpactCueTag, TEXT("GameplayCue.DBA.Skill.Impact")), HitTarget ? HitTarget : OwnerActor, ImpactLocation, 0.0f, false);
	}
}

void UDBAZodiacSkillVFXComponent_Generic::PlayProjectileVFX(FVector Start, FVector End)
{
	if (UParticleSystem* VFX = CachedVFXData.ProjectileVFX.Get())
	{
		FVector Direction = (End - Start).GetSafeNormal();
		FRotator Rotation = Direction.Rotation();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, Start, Rotation, true);
	}

	if (USoundBase* SFX = CachedVFXData.ProjectileSFX.Get())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, Start);
	}

	ExecuteSkillGameplayCue(ResolveCueTag(ProjectileCueTag, TEXT("GameplayCue.DBA.Skill.Projectile")), GetOwner(), Start, 0.0f, false);
}

void UDBAZodiacSkillVFXComponent_Generic::PlayAOEVFX(FVector Center, float Radius)
{
	const float EffectiveRadius = Radius > 0.0f ? Radius : DefaultAOEDamageRadius;
	if (UParticleSystem* VFX = CachedVFXData.AOEVFX.Get())
	{
		FRotator Rotation = FRotator::ZeroRotator;
		UParticleSystemComponent* PSystem = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), VFX, Center, Rotation, true);
		if (PSystem)
		{
			PSystem->SetVectorParameter(FName(TEXT("Radius")), FVector(EffectiveRadius));
		}
	}

	ExecuteSkillGameplayCue(ResolveCueTag(AOECueTag, TEXT("GameplayCue.DBA.Skill.AOE")), GetOwner(), Center, EffectiveRadius, false);

	if (bApplyDamageOnImpact)
	{
		TArray<AActor*> HitActors;
		ApplyAOEDamage(Center, EffectiveRadius, HitActors);
	}
}

void UDBAZodiacSkillVFXComponent_Generic::PlayChannelVFX()
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	if (UParticleSystem* VFX = CachedVFXData.ChannelVFX.Get())
	{
		FVector Location = OwnerActor->GetActorLocation();
		FRotator Rotation = OwnerActor->GetActorRotation();
		ChannelVFXComponent = UGameplayStatics::SpawnEmitterAttached(
			VFX, OwnerActor->GetRootComponent(), NAME_None, Location, Rotation, EAttachLocation::KeepRelativeOffset, true);
	}

	if (USoundBase* SFX = CachedVFXData.CastingSFX.Get())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, OwnerActor->GetActorLocation());
	}

	ExecuteSkillGameplayCue(ResolveCueTag(ChannelCueTag, TEXT("GameplayCue.DBA.Skill.Channel")), OwnerActor, OwnerActor->GetActorLocation(), 0.0f, false);
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
	if (USoundBase* SFX = CachedVFXData.CastingSFX.Get())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector);
	}
}

void UDBAZodiacSkillVFXComponent_Generic::PlayProjectileSFX()
{
	if (USoundBase* SFX = CachedVFXData.ProjectileSFX.Get())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector);
	}
}

void UDBAZodiacSkillVFXComponent_Generic::PlayImpactSFX()
{
	if (USoundBase* SFX = CachedVFXData.ImpactSFX.Get())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SFX, GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector);
	}
}

float UDBAZodiacSkillVFXComponent_Generic::CalculateSkillDamage(AActor* HitTarget, bool& bOutIsCritical) const
{
	bOutIsCritical = false;

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || BaseDamage <= 0.0f)
	{
		return 0.0f;
	}

	const EDBAElement EffectiveAttackElement = AttackElement == EDBAElement::None
		? ResolveElementFromActor(OwnerActor, EDBAElement::None)
		: AttackElement;
	const EDBAElement EffectiveDefenseElement = ResolveElementFromActor(HitTarget, FallbackDefenseElement);
	const int32 EffectiveResonanceLevel = ResonanceLevelOverride >= 0 ? ResonanceLevelOverride : ResolveResonanceLevel(OwnerActor);
	const int32 EffectiveChainLevel = ChainLevelOverride >= 0 ? ChainLevelOverride : ResolveChainLevel(OwnerActor);
	const float EffectiveCriticalRate = CriticalRateOverride >= 0.0f ? CriticalRateOverride : ResolveCriticalRate(OwnerActor, 0.0f);
	const float EffectiveCriticalMultiplier = FMath::Max(CriticalMultiplier, 1.0f);

	return UDBADamageCalculator::CalculateFinalDamage(
		BaseDamage,
		EffectiveAttackElement,
		EffectiveDefenseElement,
		EffectiveResonanceLevel,
		EffectiveChainLevel,
		ResolveDefense(HitTarget),
		EffectiveCriticalRate,
		EffectiveCriticalMultiplier,
		bOutIsCritical);
}

bool UDBAZodiacSkillVFXComponent_Generic::ApplySkillDamage(AActor* HitTarget, FVector HitLocation, float& OutFinalDamage, bool& bOutIsCritical)
{
	OutFinalDamage = 0.0f;
	bOutIsCritical = false;

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !HitTarget || HitTarget == OwnerActor)
	{
		return false;
	}

	OutFinalDamage = CalculateSkillDamage(HitTarget, bOutIsCritical);
	if (OutFinalDamage <= 0.0f)
	{
		return false;
	}

	const EDBAElement EffectiveAttackElement = AttackElement == EDBAElement::None
		? ResolveElementFromActor(OwnerActor, EDBAElement::None)
		: AttackElement;
	const FVector EffectiveHitLocation = HitLocation.IsNearlyZero() ? HitTarget->GetActorLocation() : HitLocation;
	UDBADamageCalculator::ApplyDamageToTargetWithCue(
		OwnerActor,
		HitTarget,
		OutFinalDamage,
		EffectiveAttackElement,
		bOutIsCritical,
		ResolveCueTag(ImpactCueTag, TEXT("GameplayCue.DBA.Skill.Impact")),
		EffectiveHitLocation);

	return true;
}

int32 UDBAZodiacSkillVFXComponent_Generic::ApplyAOEDamage(FVector Center, float Radius, TArray<AActor*>& OutHitActors)
{
	OutHitActors.Reset();

	UWorld* World = GetWorld();
	AActor* OwnerActor = GetOwner();
	if (!World || !OwnerActor)
	{
		return 0;
	}

	const float EffectiveRadius = Radius > 0.0f ? Radius : DefaultAOEDamageRadius;
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams(FName(TEXT("DBAZodiacSkillAOE")), false, OwnerActor);
	TArray<FOverlapResult> Overlaps;
	if (!World->OverlapMultiByObjectType(Overlaps, Center, FQuat::Identity, ObjectQueryParams, FCollisionShape::MakeSphere(EffectiveRadius), QueryParams))
	{
		return 0;
	}

	TSet<AActor*> DamagedActors;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* HitActor = Overlap.GetActor();
		if (!HitActor || HitActor == OwnerActor || DamagedActors.Contains(HitActor))
		{
			continue;
		}

		float FinalDamage = 0.0f;
		bool bIsCritical = false;
		if (ApplySkillDamage(HitActor, HitActor->GetActorLocation(), FinalDamage, bIsCritical))
		{
			DamagedActors.Add(HitActor);
			OutHitActors.Add(HitActor);
		}
	}

	return OutHitActors.Num();
}

void UDBAZodiacSkillVFXComponent_Generic::ExecuteSkillGameplayCue(const FGameplayTag& CueTag, AActor* CueTarget, FVector Location, float RawMagnitude, bool bIsCritical) const
{
	if (!CueTag.IsValid())
	{
		return;
	}

	AActor* OwnerActor = GetOwner();
	UAbilitySystemComponent* CueASC = ResolveAbilitySystemComponent(CueTarget);
	if (!CueASC)
	{
		CueASC = ResolveAbilitySystemComponent(OwnerActor);
	}
	if (!CueASC)
	{
		return;
	}

	FGameplayCueParameters CueParams;
	CueParams.Instigator = OwnerActor;
	CueParams.EffectCauser = OwnerActor;
	CueParams.SourceObject = this;
	CueParams.Location = Location.IsNearlyZero() && CueTarget ? CueTarget->GetActorLocation() : Location;
	CueParams.RawMagnitude = RawMagnitude;
	CueParams.NormalizedMagnitude = bIsCritical ? 1.0f : 0.0f;
	CueASC->ExecuteGameplayCue(CueTag, CueParams);
	if (UDBAAbilitySystemComponent* DBAASC = Cast<UDBAAbilitySystemComponent>(CueASC))
	{
		DBAASC->OnSkillCueExecuted.Broadcast(CueTag.GetTagName(), CueTarget);
	}
}
