// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "DBASkillProjectileBase.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class UAudioComponent;
class UPrimitiveComponent;
class USoundBase;
class USphereComponent;

UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBASkillProjectileBase : public AActor
{
	GENERATED_BODY()

public:
	ADBASkillProjectileBase();

	UFUNCTION(BlueprintCallable, Category = "DBA|Projectile")
	virtual void InitializeProjectile(
		FName InSkillId,
		AActor* InOwner,
		AActor* InTarget,
		float InDamage,
		float InSpeed,
		float InRadius);

	UFUNCTION(BlueprintCallable, Category = "DBA|Projectile")
	void SetProjectileProperties(float InSpeed, float InRadius, float InDamage);

	UFUNCTION(BlueprintCallable, Category = "DBA|Projectile")
	void LaunchProjectile(const FVector& Direction);

	UFUNCTION(BlueprintCallable, Category = "DBA|Projectile")
	virtual void OnProjectileHit(AActor* HitActor, FVector HitLocation);

	UFUNCTION(BlueprintCallable, Category = "DBA|Projectile")
	void SetCollisionChannel(ECollisionChannel Channel);

protected:
	UFUNCTION(NetMulticast, Reliable)
	void MulticastApplyProjectileVisuals(const FString& ProjectileVFXPath, const FString& ProjectileNiagaraVFXPath, const FString& FlySFXPath);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayImpactFeedback(const FString& ImpactVFXPath, const FString& ImpactNiagaraVFXPath, const FString& ImpactSFXPath, FVector_NetQuantize HitLocation, FRotator HitRotation);

	void ApplyProjectileVisualsLocal(const FString& ProjectileVFXPath, const FString& ProjectileNiagaraVFXPath, const FString& FlySFXPath);
	void PlayImpactFeedbackLocal(const FString& ImpactVFXPath, const FString& ImpactNiagaraVFXPath, const FString& ImpactSFXPath, const FVector& HitLocation, const FRotator& HitRotation);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UParticleSystemComponent> ProjectileVFX;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> ProjectileNiagaraVFX;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAudioComponent> ProjectileLoopAudio;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Config")
	FName SkillId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Config")
	float Damage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Config")
	float Speed = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Config")
	float Radius = 50.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|VFX")
	TSoftObjectPtr<UParticleSystem> ProjectileVFXAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|VFX")
	TSoftObjectPtr<UParticleSystem> ImpactVFXAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|VFX")
	TSoftObjectPtr<UNiagaraSystem> ProjectileNiagaraVFXAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|VFX")
	TSoftObjectPtr<UNiagaraSystem> ImpactNiagaraVFXAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|SFX")
	TSoftObjectPtr<USoundBase> FlySFXAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|SFX")
	TSoftObjectPtr<USoundBase> ImpactSFXAsset;

protected:
	UPROPERTY(Transient)
	TObjectPtr<AActor> ProjectileOwner;

	UPROPERTY(Transient)
	TObjectPtr<AActor> TargetActor;

	UFUNCTION()
	void HandleProjectileHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		FVector NormalImpulse,
		const FHitResult& Hit);

	UFUNCTION()
	void HandleProjectileOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|Projectile", meta = (DisplayName = "On Hit"))
	void BP_OnProjectileHit(AActor* HitActor, FVector HitLocation);
};
