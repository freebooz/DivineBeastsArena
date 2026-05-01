// Copyright Freebooz Games, Inc. All Rights Reserved.
// VFX/SFX 挂载组件 - 玄花灵蛇

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DBAZodiacVFXComponent_Snake.generated.h"

class UParticleSystem;
class USoundCue;
class UAnimMontage;

/**
 * UDBAZodiacVFXComponent_Snake
 * 生肖角色 VFX/SFX 挂载组件
 * 负责管理玄花灵蛇的视觉和音效资源
 */
UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = "DBA Zodiac VFX Component Snake"))
class DIVINEBEASTSARENA_API UDBAZodiacVFXComponent_Snake : public UActorComponent
{
	GENERATED_BODY()

public:
	UDBAZodiacVFXComponent_Snake();

public:
	// ==================== VFX 接口 ====================

	/** 播放攻击特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|VFX|玄花灵蛇")
	void PlayAttackVFX(AActor* Target);

	/** 播放受击特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|VFX|玄花灵蛇")
	void PlayHitVFX(AActor* Attacker);

	/** 播放移动特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|VFX|玄花灵蛇")
	void PlayMoveVFX(const FVector& Direction);

	/** 播放死亡特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|VFX|玄花灵蛇")
	void PlayDeathVFX();

	/** 播放重生特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|VFX|玄花灵蛇")
	void PlayRespawnVFX();

	// ==================== SFX 接口 ====================

	/** 播放攻击音效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SFX|玄花灵蛇")
	void PlayAttackSFX();

	/** 播放受击音效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SFX|玄花灵蛇")
	void PlayHitSFX();

	/** 播放移动音效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SFX|玄花灵蛇")
	void PlayMoveSFX();

	/** 播放死亡音效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SFX|玄花灵蛇")
	void PlayDeathSFX();

	/** 播放技能音效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SFX|玄花灵蛇")
	void PlaySkillSFX(FName SkillId);

	// ==================== 动画接口 ====================

	/** 播放攻击动画 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Animation|玄花灵蛇")
	void PlayAttackAnimation();

	/** 播放受击动画 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Animation|玄花灵蛇")
	void PlayHitAnimation();

	/** 播放移动动画 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Animation|玄花灵蛇")
	void PlayMoveAnimation(float Speed);

	/** 播放死亡动画 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Animation|玄花灵蛇")
	void PlayDeathAnimation();

	/** 播放技能动画 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Animation|玄花灵蛇")
	void PlaySkillAnimation(FName SkillId);

	/** 获取动画蓝图 */
	UFUNCTION(BlueprintPure, Category = "DBA|Animation|玄花灵蛇")
	UAnimBlueprint* GetAnimBlueprint() const;

protected:
	// ==================== VFX 资源 ====================

	/** 攻击特效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|玄花灵蛇")
	TSoftObjectPtr<UParticleSystem> AttackVFX;

	/** 受击特效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|玄花灵蛇")
	TSoftObjectPtr<UParticleSystem> HitVFX;

	/** 移动特效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|玄花灵蛇")
	TSoftObjectPtr<UParticleSystem> MoveVFX;

	/** 死亡特效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|玄花灵蛇")
	TSoftObjectPtr<UParticleSystem> DeathVFX;

	/** 重生特效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|玄花灵蛇")
	TSoftObjectPtr<UParticleSystem> RespawnVFX;

	// ==================== SFX 资源 ====================

	/** 攻击音效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SFX|玄花灵蛇")
	TSoftObjectPtr<USoundCue> AttackSFX;

	/** 受击音效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SFX|玄花灵蛇")
	TSoftObjectPtr<USoundCue> HitSFX;

	/** 移动音效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SFX|玄花灵蛇")
	TSoftObjectPtr<USoundCue> MoveSFX;

	/** 死亡音效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SFX|玄花灵蛇")
	TSoftObjectPtr<USoundCue> DeathSFX;

	// ==================== 动画资源 ====================

	/** 动画蓝图 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|玄花灵蛇")
	TSoftClassPtr<UAnimInstance> AnimBlueprintClass;

	/** 攻击动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|玄花灵蛇")
	TSoftObjectPtr<UAnimMontage> AttackMontage;

	/** 受击动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|玄花灵蛇")
	TSoftObjectPtr<UAnimMontage> HitMontage;

	/** 死亡动画 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|玄花灵蛇")
	TSoftObjectPtr<UAnimMontage> DeathMontage;

	/** 技能动画映射 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|玄花灵蛇")
	TMap<FName, TSoftObjectPtr<UAnimMontage>> SkillMontages;

	/** 角色元素类型 (用于加载对应元素特效) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	FName ElementType = FName(TEXT("Water"));
};
