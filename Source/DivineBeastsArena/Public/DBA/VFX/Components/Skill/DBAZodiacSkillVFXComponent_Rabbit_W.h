// Copyright Freebooz Games, Inc. All Rights Reserved.
// 技能VFX/SFX挂载组件 - 月华灵兔技能W

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DBAZodiacSkillVFXComponent_Rabbit_W.generated.h"

class UParticleSystem;
class USoundCue;
class UAnimMontage;

/**
 * DBAZodiacSkillVFXComponent_Rabbit_W
 * 技能VFX/SFX挂载组件
 * 负责管理技能的视觉和音效资源
 */
UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = "DBA Skill VFX Rabbit W"))
class DIVINEBEASTSARENA_API UDBAZodiacSkillVFXComponent_Rabbit_W : public UActorComponent
{
	GENERATED_BODY()

public:
	UDBAZodiacSkillVFXComponent_Rabbit_W();

public:
	// ==================== 技能特效接口 ====================

	/** 播放施法特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillVFX|月华灵兔技能W")
	void PlayCastingVFX(AActor* Target);

	/** 播放命中特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillVFX|月华灵兔技能W")
	void PlayImpactVFX(AActor* HitTarget);

	/** 播放飞行特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillVFX|月华灵兔技能W")
	void PlayProjectileVFX(FVector Start, FVector End);

	/** 播放范围特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillVFX|月华灵兔技能W")
	void PlayAOEVFX(FVector Center, float Radius);

	/** 播放引导特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillVFX|月华灵兔技能W")
	void PlayChannelVFX();

	/** 停止引导特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillVFX|月华灵兔技能W")
	void StopChannelVFX();

	// ==================== 技能音效接口 ====================

	/** 播放施法音效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillSFX|月华灵兔技能W")
	void PlayCastingSFX();

	/** 播放飞行音效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillSFX|月华灵兔技能W")
	void PlayProjectileSFX();

	/** 播放命中音效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillSFX|月华灵兔技能W")
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
