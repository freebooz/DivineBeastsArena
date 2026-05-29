// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
Readable notes:
- App: DBA_GameClient Unreal Engine client.
- Purpose: original chain lightning spell actor with jumping electric arcs and impact bursts.
- This provides a shaman-like fantasy lightning chain without copying a third-party spell exactly.
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "GameDBA/Combat/DBANiagaraSkillParameters.h"
#include "GameCore/Types/DBACommonEnums.h"
#include "DBAChainLightningSpell.generated.h"

class UNiagaraSystem;
class USceneComponent;
class USoundBase;
struct FDBAPlayableSkillRuntimeSpec;

UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBAChainLightningSpell : public AActor
{
	GENERATED_BODY()

public:
	ADBAChainLightningSpell();

	UFUNCTION(BlueprintCallable, Category = "DBA|Chain Lightning")
	void CastChainLightning(AActor* InCaster, AActor* InitialTarget);

	UFUNCTION(BlueprintCallable, Category = "DBA|Chain Lightning")
	AActor* FindInitialTarget(AActor* InCaster) const;

	void ConfigureFromSkillSpec(const FDBAPlayableSkillRuntimeSpec& Spec);
	void PreloadPresentationAssets();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Chain Lightning", meta = (ClampMin = "1"))
	int32 MaxJumps = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Chain Lightning", meta = (ClampMin = "0.0"))
	float JumpRadius = 720.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Chain Lightning", meta = (ClampMin = "0.0"))
	float BaseDamage = 38.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Chain Lightning", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DamageFalloffPerJump = 0.78f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Chain Lightning", meta = (ClampMin = "0.0"))
	float SegmentDelay = 0.075f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Chain Lightning")
	EDBAElement DamageElement = EDBAElement::Gold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Chain Lightning")
	FGameplayTag ImpactCueTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Chain Lightning|VFX")
	TSoftObjectPtr<UNiagaraSystem> ArcVFXAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Chain Lightning|VFX")
	TSoftObjectPtr<UNiagaraSystem> BranchVFXAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Chain Lightning|VFX")
	TSoftObjectPtr<UNiagaraSystem> ImpactVFXAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Chain Lightning|SFX")
	TSoftObjectPtr<USoundBase> CastSFXAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Chain Lightning|SFX")
	TSoftObjectPtr<USoundBase> FlightSFXAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Chain Lightning|SFX")
	TSoftObjectPtr<USoundBase> ImpactSFXAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Chain Lightning|VFX")
	FDBANiagaraSkillParameters NiagaraParameters;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayChainLightning(const TArray<FVector_NetQuantize>& Sources, const TArray<FVector_NetQuantize>& Targets, const TArray<float>& SegmentScales);

private:
	AActor* FindNextTarget(const FVector& FromLocation, AActor* Caster, const TSet<TObjectPtr<AActor>>& AlreadyHit) const;
	void ApplyChainDamage(AActor* Caster, AActor* Target, const FVector& HitLocation, int32 JumpIndex) const;
	void StartLocalSequence(const TArray<FVector_NetQuantize>& Sources, const TArray<FVector_NetQuantize>& Targets, const TArray<float>& SegmentScales);
	void PlayNextLocalSegment();
	void SpawnArcSegment(const FVector& Source, const FVector& Target, float SegmentScale) const;
	void SpawnImpactBurst(const FVector& Location, float SegmentScale) const;
	void PlaySFXAtLocation(const TSoftObjectPtr<USoundBase>& Asset, const FVector& Location, float Volume) const;
	FGameplayTag GetResolvedImpactCueTag() const;

	UPROPERTY(Transient)
	TArray<FVector> PendingSources;

	UPROPERTY(Transient)
	TArray<FVector> PendingTargets;

	UPROPERTY(Transient)
	TArray<float> PendingSegmentScales;

	FTimerHandle SequenceTimerHandle;
	int32 PendingSegmentIndex = 0;
};
