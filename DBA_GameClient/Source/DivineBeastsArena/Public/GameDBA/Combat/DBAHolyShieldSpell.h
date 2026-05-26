// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
Readable notes:
- App: DBA_GameClient Unreal Engine client.
- Purpose: original priest-like protective shield spell with holy barrier VFX and staged SFX.
- This is an original fantasy shield implementation, not copied from a third-party game.
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DBAHolyShieldSpell.generated.h"

class UNiagaraSystem;
class USceneComponent;
class USoundBase;
class UNiagaraComponent;
struct FDBAPlayableSkillRuntimeSpec;

UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBAHolyShieldSpell : public AActor
{
	GENERATED_BODY()

public:
	ADBAHolyShieldSpell();

	UFUNCTION(BlueprintCallable, Category = "DBA|Holy Shield")
	void CastHolyShield(AActor* InCaster, AActor* PreferredTarget = nullptr);

	void ConfigureFromSkillSpec(const FDBAPlayableSkillRuntimeSpec& Spec);
	void PreloadPresentationAssets();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Holy Shield", meta = (ClampMin = "0.0"))
	float ShieldAmount = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Holy Shield", meta = (ClampMin = "0.0"))
	float ShieldDuration = 6.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Holy Shield|VFX")
	TSoftObjectPtr<UNiagaraSystem> CastVFXAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Holy Shield|VFX")
	TSoftObjectPtr<UNiagaraSystem> BarrierVFXAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Holy Shield|VFX")
	TSoftObjectPtr<UNiagaraSystem> FlightVFXAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Holy Shield|VFX")
	TSoftObjectPtr<UNiagaraSystem> ImpactVFXAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Holy Shield|SFX")
	TSoftObjectPtr<USoundBase> CastSFXAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Holy Shield|SFX")
	TSoftObjectPtr<USoundBase> SustainSFXAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Holy Shield|SFX")
	TSoftObjectPtr<USoundBase> ImpactSFXAsset;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayShieldStart(
		AActor* TargetActor,
		FVector_NetQuantize SourceLocation,
		FVector_NetQuantize FlightTargetLocation,
		FVector_NetQuantize ImpactLocation);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayShieldEnd(FVector_NetQuantize Location);

private:
	void ApplyShield(AActor* Target);
	void ReleaseShield();
	UNiagaraComponent* SpawnVFX(const TSoftObjectPtr<UNiagaraSystem>& Asset, const FVector& Location, const FRotator& Rotation, const FVector& Scale, bool bAutoDestroy = true) const;
	UNiagaraComponent* SpawnAttachedVFX(const TSoftObjectPtr<UNiagaraSystem>& Asset, AActor* TargetActor, const FVector& RelativeOffset, const FRotator& Rotation, const FVector& Scale, bool bAutoDestroy = true) const;
	void SpawnTravelVFX(const TSoftObjectPtr<UNiagaraSystem>& Asset, const FVector& SourceLocation, const FVector& TargetLocation, float WidthScale) const;
	void PlaySFX(const TSoftObjectPtr<USoundBase>& Asset, const FVector& Location, float Volume = 1.0f) const;

	UPROPERTY(Transient)
	TObjectPtr<AActor> ShieldTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> ActiveBarrierVFX;

	float AppliedShieldAmount = 0.0f;
	FTimerHandle ShieldTimerHandle;
};
