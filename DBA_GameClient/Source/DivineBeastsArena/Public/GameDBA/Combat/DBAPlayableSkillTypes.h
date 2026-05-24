// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
Readable notes:
- App: DBA_GameClient Unreal Engine client.
- Purpose: shared playable skill runtime definitions used by characters, UI, and
  future data assets. Keep this data-only so new skills can be added without
  branching core character code.
*/

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameCore/Types/DBACommonEnums.h"
#include "DBAPlayableSkillTypes.generated.h"

class ADBABloomHealingSpell;
class ADBAChainLightningSpell;
class ADBAHolyShieldSpell;
class ADBASkillProjectileBase;
class UNiagaraSystem;
class USoundBase;

UENUM(BlueprintType)
enum class EDBAPlayableSkillEffectShape : uint8
{
	Projectile UMETA(DisplayName = "Projectile"),
	ChainLightning UMETA(DisplayName = "Chain Lightning"),
	BloomHealing UMETA(DisplayName = "Bloom Healing"),
	HolyShield UMETA(DisplayName = "Holy Shield")
};

USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAPlayableSkillRuntimeSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Skill")
	int32 SkillSlot = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Skill")
	FName SkillId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Skill")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Skill")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Skill")
	EDBAPlayableSkillEffectShape EffectShape = EDBAPlayableSkillEffectShape::Projectile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Skill", meta = (ClampMin = "0.0"))
	float Magnitude = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Skill")
	EDBAElement Element = EDBAElement::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Skill", meta = (ClampMin = "0.0"))
	float ProjectileSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Skill", meta = (ClampMin = "0.0"))
	float ProjectileRadius = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Skill", meta = (ClampMin = "0.0"))
	float Cooldown = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Skill", meta = (ClampMin = "0.0"))
	float CastVFXScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Skill")
	TSubclassOf<ADBASkillProjectileBase> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Skill")
	TSubclassOf<ADBABloomHealingSpell> BloomHealingClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Skill")
	TSubclassOf<ADBAChainLightningSpell> ChainLightningClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Skill")
	TSubclassOf<ADBAHolyShieldSpell> HolyShieldClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Cue")
	FGameplayTag ProjectileCueTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Cue")
	FGameplayTag ImpactCueTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|VFX")
	TSoftObjectPtr<UNiagaraSystem> CastNiagaraVFXAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|VFX")
	TSoftObjectPtr<UNiagaraSystem> ProjectileNiagaraVFXAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|VFX")
	TSoftObjectPtr<UNiagaraSystem> ImpactNiagaraVFXAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|SFX")
	TSoftObjectPtr<USoundBase> CastSFXAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|SFX")
	TSoftObjectPtr<USoundBase> FlySFXAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|SFX")
	TSoftObjectPtr<USoundBase> ImpactSFXAsset;
};
