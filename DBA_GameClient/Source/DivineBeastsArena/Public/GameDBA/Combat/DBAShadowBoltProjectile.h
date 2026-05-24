// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
Readable notes:
- App: DBA_GameClient Unreal Engine client.
- Purpose: original warlock-like shadow projectile with dark core, purple wake, and corrupt impact.
*/

#pragma once

#include "CoreMinimal.h"
#include "GameDBA/Combat/DBASkillProjectileBase.h"
#include "DBAShadowBoltProjectile.generated.h"

class UMaterialInterface;
class UNiagaraComponent;
class UNiagaraSystem;
class UPointLightComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBAShadowBoltProjectile : public ADBASkillProjectileBase
{
	GENERATED_BODY()

public:
	ADBAShadowBoltProjectile();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnProjectileHit(AActor* HitActor, FVector HitLocation) override;
	virtual void PreloadPresentationAssets() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ShadowCore;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ShadowHalo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> ShadowWake;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPointLightComponent> ShadowLight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Shadow")
	TObjectPtr<UMaterialInterface> ShadowMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Shadow")
	TSoftObjectPtr<UNiagaraSystem> WakeVFXAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Shadow")
	TSoftObjectPtr<UNiagaraSystem> SecondaryImpactVFXAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Shadow")
	FLinearColor ShadowColor = FLinearColor(0.44f, 0.08f, 0.95f, 1.0f);

private:
	void ApplyShadowMaterial(UStaticMeshComponent* Mesh, float EmissiveStrength, float Alpha) const;
	void ActivateWake() const;
	void SpawnImpactLayer(const TSoftObjectPtr<UNiagaraSystem>& Asset, const FVector& Location, const FRotator& Rotation, const FVector& Scale) const;

	float AgeSeconds = 0.0f;
};
