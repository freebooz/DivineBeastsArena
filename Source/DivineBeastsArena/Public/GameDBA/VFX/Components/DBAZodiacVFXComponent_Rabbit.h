// Copyright Freebooz Games, Inc. All Rights Reserved.
// VFX/SFX 鎸傝浇缁勪欢 - 鏈堝崕鐏靛厰

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DBAZodiacVFXComponent_Rabbit.generated.h"

class UParticleSystem;
class USoundBase;
class UAnimMontage;

/**
 * UDBAZodiacVFXComponent_Rabbit
 * 鐢熻倴瑙掕壊 VFX/SFX 鎸傝浇缁勪欢
 * 璐熻矗绠＄悊鏈堝崕鐏靛厰鐨勮瑙夊拰闊虫晥璧勬簮
 */
UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = "DBA Zodiac VFX Component Rabbit"))
class DIVINEBEASTSARENA_API UDBAZodiacVFXComponent_Rabbit : public UActorComponent
{
	GENERATED_BODY()

public:
	UDBAZodiacVFXComponent_Rabbit();

public:
	// ==================== VFX 鎺ュ彛 ====================

	/** 鎾斁鏀诲嚮鐗规晥 */
	UFUNCTION(BlueprintCallable, Category = "DBA|VFX|鏈堝崕鐏靛厰")
	void PlayAttackVFX(AActor* Target);

	/** 鎾斁鍙楀嚮鐗规晥 */
	UFUNCTION(BlueprintCallable, Category = "DBA|VFX|鏈堝崕鐏靛厰")
	void PlayHitVFX(AActor* Attacker);

	/** 鎾斁绉诲姩鐗规晥 */
	UFUNCTION(BlueprintCallable, Category = "DBA|VFX|鏈堝崕鐏靛厰")
	void PlayMoveVFX(const FVector& Direction);

	/** 鎾斁姝讳骸鐗规晥 */
	UFUNCTION(BlueprintCallable, Category = "DBA|VFX|鏈堝崕鐏靛厰")
	void PlayDeathVFX();

	/** 鎾斁閲嶇敓鐗规晥 */
	UFUNCTION(BlueprintCallable, Category = "DBA|VFX|鏈堝崕鐏靛厰")
	void PlayRespawnVFX();

	// ==================== SFX 鎺ュ彛 ====================

	/** 鎾斁鏀诲嚮闊虫晥 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SFX|鏈堝崕鐏靛厰")
	void PlayAttackSFX();

	/** 鎾斁鍙楀嚮闊虫晥 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SFX|鏈堝崕鐏靛厰")
	void PlayHitSFX();

	/** 鎾斁绉诲姩闊虫晥 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SFX|鏈堝崕鐏靛厰")
	void PlayMoveSFX();

	/** 鎾斁姝讳骸闊虫晥 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SFX|鏈堝崕鐏靛厰")
	void PlayDeathSFX();

	/** 鎾斁鎶€鑳介煶鏁?*/
	UFUNCTION(BlueprintCallable, Category = "DBA|SFX|鏈堝崕鐏靛厰")
	void PlaySkillSFX(FName SkillId);

	// ==================== 鍔ㄧ敾鎺ュ彛 ====================

	/** 鎾斁鏀诲嚮鍔ㄧ敾 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Animation|鏈堝崕鐏靛厰")
	void PlayAttackAnimation();

	/** 鎾斁鍙楀嚮鍔ㄧ敾 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Animation|鏈堝崕鐏靛厰")
	void PlayHitAnimation();

	/** 鎾斁绉诲姩鍔ㄧ敾 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Animation|鏈堝崕鐏靛厰")
	void PlayMoveAnimation(float Speed);

	/** 鎾斁姝讳骸鍔ㄧ敾 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Animation|鏈堝崕鐏靛厰")
	void PlayDeathAnimation();

	/** 鎾斁鎶€鑳藉姩鐢?*/
	UFUNCTION(BlueprintCallable, Category = "DBA|Animation|鏈堝崕鐏靛厰")
	void PlaySkillAnimation(FName SkillId);

	/** 鑾峰彇鍔ㄧ敾钃濆浘 */
	UFUNCTION(BlueprintPure, Category = "DBA|Animation|鏈堝崕鐏靛厰")
	UAnimBlueprint* GetAnimBlueprint() const;

protected:
	void LoadDefaultAssets();

	// ==================== VFX 璧勬簮 ====================

	/** 鏀诲嚮鐗规晥 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|鏈堝崕鐏靛厰")
	TSoftObjectPtr<UParticleSystem> AttackVFX;

	/** 鍙楀嚮鐗规晥 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|鏈堝崕鐏靛厰")
	TSoftObjectPtr<UParticleSystem> HitVFX;

	/** 绉诲姩鐗规晥 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|鏈堝崕鐏靛厰")
	TSoftObjectPtr<UParticleSystem> MoveVFX;

	/** 姝讳骸鐗规晥 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|鏈堝崕鐏靛厰")
	TSoftObjectPtr<UParticleSystem> DeathVFX;

	/** 閲嶇敓鐗规晥 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|鏈堝崕鐏靛厰")
	TSoftObjectPtr<UParticleSystem> RespawnVFX;

	// ==================== SFX 璧勬簮 ====================

	/** 鏀诲嚮闊虫晥 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SFX|鏈堝崕鐏靛厰")
	TSoftObjectPtr<USoundBase> AttackSFX;

	/** 鍙楀嚮闊虫晥 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SFX|鏈堝崕鐏靛厰")
	TSoftObjectPtr<USoundBase> HitSFX;

	/** 绉诲姩闊虫晥 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SFX|鏈堝崕鐏靛厰")
	TSoftObjectPtr<USoundBase> MoveSFX;

	/** 姝讳骸闊虫晥 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SFX|鏈堝崕鐏靛厰")
	TSoftObjectPtr<USoundBase> DeathSFX;

	// ==================== 鍔ㄧ敾璧勬簮 ====================

	/** 鍔ㄧ敾钃濆浘 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|鏈堝崕鐏靛厰")
	TSoftClassPtr<UAnimInstance> AnimBlueprintClass;

	/** 鏀诲嚮鍔ㄧ敾 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|鏈堝崕鐏靛厰")
	TSoftObjectPtr<UAnimMontage> AttackMontage;

	/** 鍙楀嚮鍔ㄧ敾 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|鏈堝崕鐏靛厰")
	TSoftObjectPtr<UAnimMontage> HitMontage;

	/** 姝讳骸鍔ㄧ敾 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|鏈堝崕鐏靛厰")
	TSoftObjectPtr<UAnimMontage> DeathMontage;

	/** 鎶€鑳藉姩鐢绘槧灏?*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|鏈堝崕鐏靛厰")
	TMap<FName, TSoftObjectPtr<UAnimMontage>> SkillMontages;

	/** 瑙掕壊鍏冪礌绫诲瀷 (鐢ㄤ簬鍔犺浇瀵瑰簲鍏冪礌鐗规晥) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	FName ElementType = FName(TEXT("Gold"));
};

