// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Combat/DBAPlayableSkillComponent.h"

#include "GameDBA/Character/DBAZodiacCharacterBase.h"
#include "GameDBA/Combat/DBABloomHealingSpell.h"
#include "GameDBA/Combat/DBAChainLightningSpell.h"
#include "GameDBA/Combat/DBAFireballProjectile.h"
#include "GameDBA/Combat/DBAFrostShardProjectile.h"
#include "GameDBA/Combat/DBAHolyShieldSpell.h"
#include "GameDBA/Combat/DBAShadowBoltProjectile.h"
#include "GameDBA/Services/DBASkillGroupGeneratorSubsystem.h"
#include "GameCore/Core/DBALogChannels.h"

namespace
{
	EDBAZodiac ToPlayableSkillCommonZodiac(EDBAZodiacType ZodiacType)
	{
		switch (ZodiacType)
		{
		case EDBAZodiacType::Rat: return EDBAZodiac::Rat;
		case EDBAZodiacType::Ox: return EDBAZodiac::Ox;
		case EDBAZodiacType::Tiger: return EDBAZodiac::Tiger;
		case EDBAZodiacType::Rabbit: return EDBAZodiac::Rabbit;
		case EDBAZodiacType::Dragon: return EDBAZodiac::Dragon;
		case EDBAZodiacType::Snake: return EDBAZodiac::Snake;
		case EDBAZodiacType::Horse: return EDBAZodiac::Horse;
		case EDBAZodiacType::Goat: return EDBAZodiac::Goat;
		case EDBAZodiacType::Monkey: return EDBAZodiac::Monkey;
		case EDBAZodiacType::Rooster: return EDBAZodiac::Rooster;
		case EDBAZodiacType::Dog: return EDBAZodiac::Dog;
		case EDBAZodiacType::Pig: return EDBAZodiac::Pig;
		default: return EDBAZodiac::Rat;
		}
	}

	EDBAElement ToPlayableSkillCommonElement(EDBAElementType ElementType)
	{
		switch (ElementType)
		{
		case EDBAElementType::Fire: return EDBAElement::Fire;
		case EDBAElementType::Water: return EDBAElement::Water;
		case EDBAElementType::Wood: return EDBAElement::Wood;
		case EDBAElementType::Metal: return EDBAElement::Gold;
		case EDBAElementType::Earth: return EDBAElement::Earth;
		default: return EDBAElement::Fire;
		}
	}

	TSoftObjectPtr<UNiagaraSystem> NiagaraAsset(const TCHAR* Path)
	{
		return TSoftObjectPtr<UNiagaraSystem>(FSoftObjectPath(Path));
	}

	TSoftObjectPtr<USoundBase> SoundAsset(const TCHAR* Path)
	{
		return TSoftObjectPtr<USoundBase>(FSoftObjectPath(Path));
	}

	FGameplayTag OptionalTag(const TCHAR* TagName)
	{
		return TagName && FCString::Strlen(TagName) > 0
			? FGameplayTag::RequestGameplayTag(FName(TagName), false)
			: FGameplayTag();
	}

	FDBAPlayableSkillRuntimeSpec MakeSkill(
		int32 SkillSlot,
		FName SkillId,
		const TCHAR* DisplayName,
		EDBAPlayableSkillEffectShape Shape,
		float Magnitude,
		EDBAElement Element,
		float ProjectileSpeed,
		float ProjectileRadius,
		float Cooldown,
		float CastVFXScale)
	{
		FDBAPlayableSkillRuntimeSpec Spec;
		Spec.SkillSlot = SkillSlot;
		Spec.SkillId = SkillId;
		Spec.DisplayName = FText::FromString(DisplayName);
		Spec.EffectShape = Shape;
		Spec.Magnitude = Magnitude;
		Spec.Element = Element;
		Spec.ProjectileSpeed = ProjectileSpeed;
		Spec.ProjectileRadius = ProjectileRadius;
		Spec.Cooldown = Cooldown;
		Spec.CastVFXScale = CastVFXScale;
		Spec.ProjectileCueTag = OptionalTag(TEXT("GameplayCue.DBA.Skill.Projectile"));
		Spec.ImpactCueTag = OptionalTag(TEXT("GameplayCue.DBA.Skill.Impact"));
		return Spec;
	}
}

UDBAPlayableSkillComponent::UDBAPlayableSkillComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	ResetToDefaultSkillSpecs();
}

void UDBAPlayableSkillComponent::BeginPlay()
{
	Super::BeginPlay();

	TArray<FString> ValidationErrors;
	if (!ValidateEffectiveSkillSpecs(ValidationErrors))
	{
		for (const FString& ValidationError : ValidationErrors)
		{
			UE_LOG(LogDBACombat, Warning, TEXT("[DBAPlayableSkillComponent] Playable skill catalog validation failed: Owner=%s Error=%s"), *GetNameSafe(GetOwner()), *ValidationError);
		}
	}
}

bool UDBAPlayableSkillComponent::GetSkillSpec(int32 SkillSlot, FDBAPlayableSkillRuntimeSpec& OutSpec) const
{
	TArray<FDBAPlayableSkillRuntimeSpec> EffectiveSpecs;
	BuildEffectiveSkillSpecs(EffectiveSpecs);

	for (const FDBAPlayableSkillRuntimeSpec& Spec : EffectiveSpecs)
	{
		if (Spec.SkillSlot == SkillSlot)
		{
			OutSpec = Spec;
			if (bResolveSkillIdsFromEquippedSkillGroup)
			{
				OutSpec.SkillId = ResolveEquippedSkillId(SkillSlot, Spec.SkillId);
			}
			return true;
		}
	}

	return false;
}

TArray<FDBAPlayableSkillRuntimeSpec> UDBAPlayableSkillComponent::GetAllSkillSpecs() const
{
	TArray<FDBAPlayableSkillRuntimeSpec> EffectiveSpecs;
	BuildEffectiveSkillSpecs(EffectiveSpecs);

	TArray<FDBAPlayableSkillRuntimeSpec> ResolvedSpecs;
	ResolvedSpecs.Reserve(EffectiveSpecs.Num());
	for (const FDBAPlayableSkillRuntimeSpec& Spec : EffectiveSpecs)
	{
		FDBAPlayableSkillRuntimeSpec ResolvedSpec = Spec;
		if (bResolveSkillIdsFromEquippedSkillGroup)
		{
			ResolvedSpec.SkillId = ResolveEquippedSkillId(Spec.SkillSlot, Spec.SkillId);
		}
		ResolvedSpecs.Add(ResolvedSpec);
	}
	return ResolvedSpecs;
}

void UDBAPlayableSkillComponent::SetSkillSpec(int32 SkillSlot, const FDBAPlayableSkillRuntimeSpec& InSpec)
{
	FDBAPlayableSkillRuntimeSpec NormalizedSpec = InSpec;
	NormalizedSpec.SkillSlot = SkillSlot;

	for (FDBAPlayableSkillRuntimeSpec& Spec : SkillSpecs)
	{
		if (Spec.SkillSlot == SkillSlot)
		{
			Spec = NormalizedSpec;
			return;
		}
	}

	SkillSpecs.Add(NormalizedSpec);
	SkillSpecs.Sort([](const FDBAPlayableSkillRuntimeSpec& Left, const FDBAPlayableSkillRuntimeSpec& Right)
	{
		return Left.SkillSlot < Right.SkillSlot;
	});
}

void UDBAPlayableSkillComponent::ResetToDefaultSkillSpecs()
{
	SkillSpecs.Reset();

	FDBAPlayableSkillRuntimeSpec Fireball = MakeSkill(1, TEXT("Lobby.Skill01.MageFireball"), TEXT("Mage Fireball"), EDBAPlayableSkillEffectShape::Projectile, 42.0f, EDBAElement::Fire, 1580.0f, 46.0f, 3.0f, 1.12f);
	Fireball.ProjectileClass = ADBAFireballProjectile::StaticClass();
	Fireball.CastNiagaraVFXAsset = NiagaraAsset(TEXT("/Game/DBA/VFX/Abilities/FireLion/NS_FireLion_Q_FlameClaw_Slash.NS_FireLion_Q_FlameClaw_Slash"));
	Fireball.ProjectileNiagaraVFXAsset = NiagaraAsset(TEXT("/Game/DBA/VFX/Fireball/NS_DBA_Fireball_Projectile.NS_DBA_Fireball_Projectile"));
	Fireball.ImpactNiagaraVFXAsset = NiagaraAsset(TEXT("/Game/DBA/VFX/Fireball/NS_DBA_Fireball_Impact.NS_DBA_Fireball_Impact"));
	Fireball.CastSFXAsset = SoundAsset(TEXT("/Game/DBA/Audio/SFX/Downloaded/ClassMagic/SFX_MageFireball_PreCast.SFX_MageFireball_PreCast"));
	Fireball.FlySFXAsset = SoundAsset(TEXT("/Game/DBA/Audio/SFX/Downloaded/ClassMagic/SFX_MageFireball_Flight.SFX_MageFireball_Flight"));
	Fireball.ImpactSFXAsset = SoundAsset(TEXT("/Game/DBA/Audio/SFX/Downloaded/ClassMagic/SFX_MageFireball_Impact.SFX_MageFireball_Impact"));
	SkillSpecs.Add(Fireball);

	FDBAPlayableSkillRuntimeSpec Frost = MakeSkill(2, TEXT("Lobby.Skill02.FrostShard"), TEXT("Frost Shard"), EDBAPlayableSkillEffectShape::Projectile, 32.0f, EDBAElement::Water, 1840.0f, 38.0f, 4.5f, 1.15f);
	Frost.ProjectileClass = ADBAFrostShardProjectile::StaticClass();
	Frost.CastNiagaraVFXAsset = NiagaraAsset(TEXT("/Game/ProjectileHitVFX/NS/NS_IceCrystal.NS_IceCrystal"));
	Frost.ProjectileNiagaraVFXAsset = NiagaraAsset(TEXT("/Game/ProjectileHitVFX/NS/NS_IceDart.NS_IceDart"));
	Frost.ImpactNiagaraVFXAsset = NiagaraAsset(TEXT("/Game/ProjectileHitVFX/NS/NS_Hit_Ice_01.NS_Hit_Ice_01"));
	Frost.CastSFXAsset = SoundAsset(TEXT("/Game/DBA/Audio/SFX/Downloaded/Magic/SFX_FrostShard_PreCast.SFX_FrostShard_PreCast"));
	Frost.FlySFXAsset = SoundAsset(TEXT("/Game/DBA/Audio/SFX/Downloaded/Magic/SFX_FrostShard_Flight.SFX_FrostShard_Flight"));
	Frost.ImpactSFXAsset = SoundAsset(TEXT("/Game/DBA/Audio/SFX/Downloaded/Magic/SFX_FrostShard_Impact.SFX_FrostShard_Impact"));
	SkillSpecs.Add(Frost);

	FDBAPlayableSkillRuntimeSpec Bloom = MakeSkill(3, TEXT("Lobby.Skill03.BloomHealing"), TEXT("Bloom Healing"), EDBAPlayableSkillEffectShape::BloomHealing, 115.0f, EDBAElement::Wood, 0.0f, 0.0f, 5.5f, 1.2f);
	Bloom.BloomHealingClass = ADBABloomHealingSpell::StaticClass();
	Bloom.CastNiagaraVFXAsset = NiagaraAsset(TEXT("/Game/DBA/VFX/Abilities/WoodCrane/NS_WoodCrane_Q_HealingGrove_Area.NS_WoodCrane_Q_HealingGrove_Area"));
	Bloom.ProjectileNiagaraVFXAsset = NiagaraAsset(TEXT("/Game/DBA/VFX/Common/Impact/NS_Impact_Heal_Burst.NS_Impact_Heal_Burst"));
	Bloom.ImpactNiagaraVFXAsset = NiagaraAsset(TEXT("/Game/DBA/VFX/Abilities/WoodCrane/NS_WoodCrane_Q_HealingBurst_Impact.NS_WoodCrane_Q_HealingBurst_Impact"));
	Bloom.CastSFXAsset = SoundAsset(TEXT("/Game/DBA/Audio/SFX/Downloaded/Magic/SFX_BloomHealing_PreCast.SFX_BloomHealing_PreCast"));
	Bloom.FlySFXAsset = SoundAsset(TEXT("/Game/DBA/Audio/SFX/Downloaded/Magic/SFX_BloomHealing_Flight.SFX_BloomHealing_Flight"));
	Bloom.ImpactSFXAsset = SoundAsset(TEXT("/Game/DBA/Audio/SFX/Downloaded/Magic/SFX_BloomHealing_Impact.SFX_BloomHealing_Impact"));
	SkillSpecs.Add(Bloom);

	FDBAPlayableSkillRuntimeSpec Chain = MakeSkill(4, TEXT("Lobby.Skill04.ChainLightning"), TEXT("Chain Lightning"), EDBAPlayableSkillEffectShape::ChainLightning, 38.0f, EDBAElement::Gold, 0.0f, 0.0f, 6.0f, 1.2f);
	Chain.ChainLightningClass = ADBAChainLightningSpell::StaticClass();
	Chain.CastNiagaraVFXAsset = NiagaraAsset(TEXT("/Game/ProjectileHitVFX/NS/NS_Hit_Eletric_01.NS_Hit_Eletric_01"));
	Chain.ProjectileNiagaraVFXAsset = NiagaraAsset(TEXT("/Game/ProjectileHitVFX/NS/NS_ThunderBolt.NS_ThunderBolt"));
	Chain.ImpactNiagaraVFXAsset = NiagaraAsset(TEXT("/Game/ProjectileHitVFX/NS/NS_Hit_Thunder.NS_Hit_Thunder"));
	Chain.CastSFXAsset = SoundAsset(TEXT("/Game/DBA/Audio/SFX/Downloaded/Magic/SFX_ChainLightning_PreCast.SFX_ChainLightning_PreCast"));
	Chain.FlySFXAsset = SoundAsset(TEXT("/Game/DBA/Audio/SFX/Downloaded/Magic/SFX_ChainLightning_Flight.SFX_ChainLightning_Flight"));
	Chain.ImpactSFXAsset = SoundAsset(TEXT("/Game/DBA/Audio/SFX/Downloaded/Magic/SFX_ChainLightning_Impact.SFX_ChainLightning_Impact"));
	SkillSpecs.Add(Chain);

	FDBAPlayableSkillRuntimeSpec Shield = MakeSkill(5, TEXT("Lobby.Skill05.PriestShield"), TEXT("Priest Shield"), EDBAPlayableSkillEffectShape::HolyShield, 180.0f, EDBAElement::Wood, 0.0f, 0.0f, 8.0f, 1.22f);
	Shield.HolyShieldClass = ADBAHolyShieldSpell::StaticClass();
	Shield.ProjectileNiagaraVFXAsset = NiagaraAsset(TEXT("/Game/DBA/VFX/Common/Status/NS_Status_Shielded.NS_Status_Shielded"));
	Shield.ImpactNiagaraVFXAsset = NiagaraAsset(TEXT("/Game/ProjectileHitVFX/NS/NS_HolyEnergy.NS_HolyEnergy"));
	Shield.CastSFXAsset = SoundAsset(TEXT("/Game/DBA/Audio/SFX/Downloaded/ClassMagic/SFX_PriestShield_PreCast.SFX_PriestShield_PreCast"));
	Shield.FlySFXAsset = SoundAsset(TEXT("/Game/DBA/Audio/SFX/Downloaded/ClassMagic/SFX_PriestShield_Flight.SFX_PriestShield_Flight"));
	Shield.ImpactSFXAsset = SoundAsset(TEXT("/Game/DBA/Audio/SFX/Downloaded/ClassMagic/SFX_PriestShield_Impact.SFX_PriestShield_Impact"));
	SkillSpecs.Add(Shield);

	FDBAPlayableSkillRuntimeSpec Shadow = MakeSkill(6, TEXT("Lobby.Skill06.ShadowBolt"), TEXT("Shadow Bolt"), EDBAPlayableSkillEffectShape::Projectile, 44.0f, EDBAElement::Gold, 1580.0f, 40.0f, 4.8f, 1.1f);
	Shadow.ProjectileClass = ADBAShadowBoltProjectile::StaticClass();
	Shadow.CastNiagaraVFXAsset = NiagaraAsset(TEXT("/Game/ProjectileHitVFX/NS/NS_Hit_Magic.NS_Hit_Magic"));
	Shadow.ProjectileNiagaraVFXAsset = NiagaraAsset(TEXT("/Game/ProjectileHitVFX/NS/NS_PoisonSkullFish.NS_PoisonSkullFish"));
	Shadow.ImpactNiagaraVFXAsset = NiagaraAsset(TEXT("/Game/ProjectileHitVFX/NS/NS_Hit_Poison.NS_Hit_Poison"));
	Shadow.CastSFXAsset = SoundAsset(TEXT("/Game/DBA/Audio/SFX/Downloaded/ClassMagic/SFX_ShadowBolt_PreCast.SFX_ShadowBolt_PreCast"));
	Shadow.FlySFXAsset = SoundAsset(TEXT("/Game/DBA/Audio/SFX/Downloaded/ClassMagic/SFX_ShadowBolt_Flight.SFX_ShadowBolt_Flight"));
	Shadow.ImpactSFXAsset = SoundAsset(TEXT("/Game/DBA/Audio/SFX/Downloaded/ClassMagic/SFX_ShadowBolt_Impact.SFX_ShadowBolt_Impact"));
	SkillSpecs.Add(Shadow);
}

void UDBAPlayableSkillComponent::SetSkillCatalog(UDBAPlayableSkillCatalogDataAsset* InSkillCatalog)
{
	SkillCatalog = InSkillCatalog;
}

void UDBAPlayableSkillComponent::SetAppendDefaultSkillsWhenCatalogMissingSlots(bool bInAppendDefaults)
{
	bAppendDefaultSkillsWhenCatalogMissingSlots = bInAppendDefaults;
}

bool UDBAPlayableSkillComponent::ValidateEffectiveSkillSpecs(TArray<FString>& OutErrors) const
{
	TArray<FDBAPlayableSkillRuntimeSpec> EffectiveSpecs;
	BuildEffectiveSkillSpecs(EffectiveSpecs);
	return UDBAPlayableSkillCatalogDataAsset::ValidateSkillSpecs(EffectiveSpecs, OutErrors);
}

FDBAPlayableSkillCatalogSummary UDBAPlayableSkillComponent::GetSkillCatalogSummary() const
{
	FDBAPlayableSkillCatalogSummary Summary;
	Summary.Source = SkillCatalog
		? (bAppendDefaultSkillsWhenCatalogMissingSlots ? EDBAPlayableSkillCatalogSource::DataAssetWithDefaults : EDBAPlayableSkillCatalogSource::DataAssetOnly)
		: EDBAPlayableSkillCatalogSource::BuiltInDefaults;
	Summary.CatalogId = SkillCatalog ? SkillCatalog->CatalogId : FName(TEXT("BuiltInDefaults"));
	Summary.ConfiguredCatalogSkillCount = SkillCatalog ? SkillCatalog->GetAllSkillSpecs().Num() : 0;
	Summary.bAppendsBuiltInDefaults = !SkillCatalog || bAppendDefaultSkillsWhenCatalogMissingSlots;

	TArray<FDBAPlayableSkillRuntimeSpec> EffectiveSpecs;
	BuildEffectiveSkillSpecs(EffectiveSpecs);
	Summary.SkillCount = EffectiveSpecs.Num();
	Summary.bIsValid = UDBAPlayableSkillCatalogDataAsset::ValidateSkillSpecs(EffectiveSpecs, Summary.ValidationErrors);
	return Summary;
}

void UDBAPlayableSkillComponent::BuildEffectiveSkillSpecs(TArray<FDBAPlayableSkillRuntimeSpec>& OutSpecs) const
{
	OutSpecs.Reset();

	if (!SkillCatalog || bAppendDefaultSkillsWhenCatalogMissingSlots)
	{
		OutSpecs = SkillSpecs;
	}

	if (SkillCatalog)
	{
		const TArray<FDBAPlayableSkillRuntimeSpec> CatalogSpecs = SkillCatalog->GetAllSkillSpecs();
		for (const FDBAPlayableSkillRuntimeSpec& CatalogSpec : CatalogSpecs)
		{
			if (FDBAPlayableSkillRuntimeSpec* ExistingSpec = OutSpecs.FindByPredicate([&CatalogSpec](const FDBAPlayableSkillRuntimeSpec& Candidate)
				{
					return Candidate.SkillSlot == CatalogSpec.SkillSlot;
				}))
			{
				*ExistingSpec = CatalogSpec;
			}
			else
			{
				OutSpecs.Add(CatalogSpec);
			}
		}
	}

	OutSpecs.Sort([](const FDBAPlayableSkillRuntimeSpec& Left, const FDBAPlayableSkillRuntimeSpec& Right)
	{
		return Left.SkillSlot < Right.SkillSlot;
	});
}

FName UDBAPlayableSkillComponent::ResolveEquippedSkillId(int32 SkillSlot, FName FallbackSkillId) const
{
	const ADBAZodiacCharacterBase* Character = Cast<ADBAZodiacCharacterBase>(GetOwner());
	if (!Character || !Character->GetWorld())
	{
		return FallbackSkillId;
	}

	const UGameInstance* GameInstance = Character->GetWorld()->GetGameInstance();
	const UDBASkillGroupGeneratorSubsystem* SkillGroups = GameInstance
		? GameInstance->GetSubsystem<UDBASkillGroupGeneratorSubsystem>()
		: nullptr;
	if (!SkillGroups)
	{
		return FallbackSkillId;
	}

	FDBAZodiacElementFixedSkillGroupRow SkillGroup;
	if (!SkillGroups->GetSkillGroup(ToPlayableSkillCommonZodiac(Character->GetZodiacType()), ToPlayableSkillCommonElement(Character->GetElementType()), SkillGroup))
	{
		return FallbackSkillId;
	}

	switch (SkillSlot)
	{
	case 1: return SkillGroup.ElementSkill1Id.IsNone() ? FallbackSkillId : SkillGroup.ElementSkill1Id;
	case 2: return SkillGroup.ElementSkill2Id.IsNone() ? FallbackSkillId : SkillGroup.ElementSkill2Id;
	case 3: return SkillGroup.ElementSkill3Id.IsNone() ? FallbackSkillId : SkillGroup.ElementSkill3Id;
	case 4: return SkillGroup.ElementSkill4Id.IsNone() ? FallbackSkillId : SkillGroup.ElementSkill4Id;
	case 5: return SkillGroup.ZodiacUltimateSkillId.IsNone() ? FallbackSkillId : SkillGroup.ZodiacUltimateSkillId;
	default: return FallbackSkillId;
	}
}
