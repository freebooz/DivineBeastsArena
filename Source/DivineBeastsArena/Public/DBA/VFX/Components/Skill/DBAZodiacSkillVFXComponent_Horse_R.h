// Copyright Freebooz Games, Inc. All Rights Reserved.
// 技能VFX/SFX挂载组件 - 踏风天驹终极技能

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DBAZodiacSkillVFXComponent_Horse_R.generated.h"

class UParticleSystem;
class USoundCue;
class UAnimMontage;

/**
 * DBAZodiacSkillVFXComponent_Horse_R
 * 技能VFX/SFX挂载组件
 * 负责管理技能的视觉和音效资源
 */
UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = "DBA Skill VFX Horse R"))
class DIVINEBEASTSARENA_API UDBAZodiacSkillVFXComponent_Horse_R : public UActorComponent
{
	GENERATED_BODY()

public:
	UDBAZodiacSkillVFXComponent_Horse_R();

public:
	// ==================== 技能特效接口 ====================

	/** 播放施法特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillVFX|踏风天驹终极技能")
	void PlayCastingVFX(AActor* Target);

	/** 播放命中特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillVFX|踏风天驹终极技能")
	void PlayImpactVFX(AActor* HitTarget);

	/** 播放飞行特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillVFX|踏风天驹终极技能")
	void PlayProjectileVFX(FVector Start, FVector End);

	/** 播放范围特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillVFX|踏风天驹终极技能")
	void PlayAOEVFX(FVector Center, float Radius);

	/** 播放引导特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillVFX|踏风天驹终极技能")
	void PlayChannelVFX();

	/** 停止引导特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillVFX|踏风天驹终极技能")
	void StopChannelVFX();

	// ==================== 技能音效接口 ====================

	/** 播放施法音效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillSFX|踏风天驹终极技能")
	void PlayCastingSFX();

	/** 播放飞行音效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillSFX|踏风天驹终极技能")
	void PlayProjectileSFX();

	/** 播放命中音效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillSFX|踏风天驹终极技能")
	void PlayImpactSFX();

protected:
	// ==================== 特效资源 ====================

	/** 施法特效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Skill")
	TSoftObjectPtr<UParticleSystem> CastingVFX;

	/** 命中特效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Skill")
	TSoftObjectPtr<UParticleSystem> ImpactVFX;

	/** 飞行弹道特效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Skill")
	TSoftObjectPtr<UParticleSystem> ProjectileVFX;

	/** 范围爆炸特效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Skill")
	TSoftObjectPtr<UParticleSystem> AOEVFX;

	/** 引导持续特效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Skill")
	TSoftObjectPtr<UParticleSystem> ChannelVFX;

	/** 增益特效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Skill")
	TSoftObjectPtr<UParticleSystem> BuffVFX;

	/** 减益特效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Skill")
	TSoftObjectPtr<UParticleSystem> DebuffVFX;

	// ==================== 音效资源 ====================

	/** 施法音效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SFX|Skill")
	TSoftObjectPtr<USoundCue> CastingSFX;

	/** 飞行弹道音效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SFX|Skill")
	TSoftObjectPtr<USoundCue> ProjectileSFX;

	/** 命中音效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SFX|Skill")
	TSoftObjectPtr<USoundCue> ImpactSFX;

	// ==================== 动画资源 ====================

	/** 施法动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Skill")
	TSoftObjectPtr<UAnimMontage> CastingMontage;

	/** 命中动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Skill")
	TSoftObjectPtr<UAnimMontage> ImpactMontage;

protected:
	/** 引导特效的粒子组件 */
	UPROPERTY()
	TObjectPtr<UParticleSystemComponent> ChannelVFXComponent;
};
