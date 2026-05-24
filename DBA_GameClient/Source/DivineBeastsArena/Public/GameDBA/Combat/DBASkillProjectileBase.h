// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameplayTagContainer.h"
#include "GameCore/Types/DBACommonEnums.h"
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

	virtual void PreloadPresentationAssets();

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Damage")
	EDBAElement DamageElement = EDBAElement::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Damage", meta = (ClampMin = "-1", ClampMax = "4"))
	int32 ResonanceLevelOverride = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Damage", meta = (ClampMin = "-1", ClampMax = "10"))
	int32 ChainLevelOverride = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Damage", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float CriticalRateOverride = -1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Damage", meta = (ClampMin = "1.0"))
	float CriticalMultiplier = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Cue")
	FGameplayTag ProjectileCueTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Cue")
	FGameplayTag ImpactCueTag;

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

	bool bProjectileHitProcessed = false;

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
