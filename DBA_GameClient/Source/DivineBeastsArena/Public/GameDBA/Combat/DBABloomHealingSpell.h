// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
Readable notes:
- App: DBA_GameClient Unreal Engine client.
- Purpose: original nature bloom healing spell with growth, bloom, and healing pulse visuals.
- This is a druid-like fantasy healing effect, not a frame-accurate clone of any third-party spell.
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DBABloomHealingSpell.generated.h"

class UNiagaraSystem;
class USceneComponent;
class USoundBase;
struct FDBAPlayableSkillRuntimeSpec;

UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBABloomHealingSpell : public AActor
{
	GENERATED_BODY()

public:
	ADBABloomHealingSpell();

	UFUNCTION(BlueprintCallable, Category = "DBA|Bloom Healing")
	void CastBloomHealing(AActor* InCaster, AActor* PreferredTarget = nullptr);

	void ConfigureFromSkillSpec(const FDBAPlayableSkillRuntimeSpec& Spec);
	void PreloadPresentationAssets();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Bloom Healing", meta = (ClampMin = "0.0"))
	float HealAmount = 115.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Bloom Healing", meta = (ClampMin = "0.0"))
	float BloomDelay = 0.42f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Bloom Healing", meta = (ClampMin = "0.0"))
	float HealRadius = 380.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Bloom Healing", meta = (ClampMin = "1"))
	int32 MaxHealTargets = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Bloom Healing|VFX")
	TSoftObjectPtr<UNiagaraSystem> SeedVFXAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Bloom Healing|VFX")
	TSoftObjectPtr<UNiagaraSystem> GroveVFXAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Bloom Healing|VFX")
	TSoftObjectPtr<UNiagaraSystem> BloomVFXAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Bloom Healing|VFX")
	TSoftObjectPtr<UNiagaraSystem> PulseVFXAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Bloom Healing|SFX")
	TSoftObjectPtr<USoundBase> CastSFXAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Bloom Healing|SFX")
	TSoftObjectPtr<USoundBase> FlightSFXAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Bloom Healing|SFX")
	TSoftObjectPtr<USoundBase> BloomSFXAsset;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayBloomStart(
		AActor* AnchorActor,
		FVector_NetQuantize SourceLocation,
		FVector_NetQuantize FlightTargetLocation,
		FVector_NetQuantize BloomLocation);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayBloomRelease(FVector_NetQuantize Location, const TArray<FVector_NetQuantize>& HealTargetLocations);

private:
	void ReleaseBloom();
	void ApplyHealing(AActor* Target) const;
	void SpawnVFX(const TSoftObjectPtr<UNiagaraSystem>& Asset, const FVector& Location, const FRotator& Rotation, const FVector& Scale) const;
	void SpawnAttachedVFX(const TSoftObjectPtr<UNiagaraSystem>& Asset, AActor* AnchorActor, const FVector& RelativeOffset, const FRotator& Rotation, const FVector& Scale) const;
	void SpawnTravelVFX(const TSoftObjectPtr<UNiagaraSystem>& Asset, const FVector& SourceLocation, const FVector& TargetLocation, float WidthScale) const;
	void PlaySFX(const TSoftObjectPtr<USoundBase>& Asset, const FVector& Location, float Volume = 1.0f) const;
	TArray<AActor*> ResolveHealTargets(AActor* Caster, AActor* PreferredTarget) const;

	UPROPERTY(Transient)
	TObjectPtr<AActor> CachedCaster;

	UPROPERTY(Transient)
	TObjectPtr<AActor> CachedPreferredTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> SceneRoot;

	FTimerHandle BloomTimerHandle;
};
