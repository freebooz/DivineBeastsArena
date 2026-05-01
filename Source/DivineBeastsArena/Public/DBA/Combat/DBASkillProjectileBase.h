// Copyright Freebooz Games, Inc. All Rights Reserved.
// 技能投射物基类

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DBASkillProjectileBase.generated.h"

class UProjectileMovementComponent;
class UParticleSystemComponent;
class USoundCue;

/**
 * DBASkillProjectileBase
 * 技能投射物基类
 * 用于实现弹道类技能，如飞行道具、弹射物等
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBASkillProjectileBase : public AActor
{
	GENERATED_BODY()

public:
	ADBASkillProjectileBase();

public:
	// ==================== 初始化 ====================

	/** 初始化投射物 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Projectile")
	void InitializeProjectile(
		FName InSkillId,
		AActor* InOwner,
		AActor* InTarget,
		float InDamage,
		float InSpeed,
		float InRadius);

	/** 设置投射物属性 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Projectile")
	void SetProjectileProperties(float Speed, float Radius, float Damage);

public:
	// ==================== 碰撞处理 ====================

	/** 命中目标时调用 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Projectile")
	virtual void OnProjectileHit(AActor* HitActor, FVector HitLocation);

	/** 设置碰撞通道 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Projectile")
	void SetCollisionChannel(ECollisionChannel Channel);

protected:
	// ==================== 组件 ====================

	/** 投射物移动组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	/** 飞行特效组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UParticleSystemComponent> ProjectileVFX;

public:
	// ==================== 配置属性 ====================

	/** 技能ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Config")
	FName SkillId = NAME_None;

	/** 伤害值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Config")
	float Damage = 0.0f;

	/** 飞行速度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Config")
	float Speed = 1000.0f;

	/** 影响半径 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Config")
	float Radius = 50.0f;

	/** 飞行特效资源 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|VFX")
	TSoftObjectPtr<UParticleSystem> ProjectileVFXAsset;

	/** 命中特效资源 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|VFX")
	TSoftObjectPtr<UParticleSystem> ImpactVFXAsset;

	/** 飞行音效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|SFX")
	TSoftObjectPtr<USoundCue> FlySFXAsset;

	/** 命中音效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|SFX")
	TSoftObjectPtr<USoundCue> ImpactSFXAsset;

protected:
	/** 投射物拥有者 */
	UPROPERTY()
	TObjectPtr<AActor> ProjectileOwner;

	/** 目标Actor */
	UPROPERTY()
	TObjectPtr<AActor> TargetActor;

protected:
	// ==================== 事件 ====================

	/** 命中事件 - 可在子类中重写 */
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|Projectile", meta = (DisplayName = "On Hit"))
	void BP_OnProjectileHit(AActor* HitActor, FVector HitLocation);
};