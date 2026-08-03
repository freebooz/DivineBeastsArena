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
#include "GameDBA/Presentation/VFX/DBANiagaraSkillParameters.h"
#include "GameCore/Types/DBACommonEnums.h"
#include "DBAPlayableSkillTypes.generated.h"

class ADBABloomHealingSpell;
class ADBAChainLightningSpell;
class ADBAHolyShieldSpell;
class ADBASkillProjectileBase;
class UNiagaraSystem;
class USoundBase;
class UTexture2D;

UENUM(BlueprintType)
enum class EDBAPlayableSkillEffectShape : uint8
{
	Projectile UMETA(DisplayName = "Projectile"),
	ChainLightning UMETA(DisplayName = "Chain Lightning"),
	BloomHealing UMETA(DisplayName = "Bloom Healing"),
	HolyShield UMETA(DisplayName = "Holy Shield")
};

UENUM(BlueprintType)
enum class EDBAPlayableSkillCatalogSource : uint8
{
	BuiltInDefaults UMETA(DisplayName = "Built-In Defaults"),
	DataAssetWithDefaults UMETA(DisplayName = "Data Asset With Defaults"),
	DataAssetOnly UMETA(DisplayName = "Data Asset Only")
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
	TSoftObjectPtr<UTexture2D> Icon;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Niagara")
	FDBANiagaraSkillParameters NiagaraParameters;

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

USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAPlayableSkillCatalogSummary
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Skill")
	EDBAPlayableSkillCatalogSource Source = EDBAPlayableSkillCatalogSource::BuiltInDefaults;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Skill")
	FName CatalogId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Skill")
	int32 SkillCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Skill")
	int32 ConfiguredCatalogSkillCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Skill")
	bool bAppendsBuiltInDefaults = true;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Skill")
	bool bIsValid = false;

	UPROPERTY(BlueprintReadOnly, Category = "DBA|Skill")
	TArray<FString> ValidationErrors;
};
