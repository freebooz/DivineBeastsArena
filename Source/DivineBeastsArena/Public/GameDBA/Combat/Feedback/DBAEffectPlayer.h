// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "DBAEffectPlayer.generated.h"

class UDBAEffectTableManager;
class UDBAFloatingDamageComponent;
class UNiagaraSystem;
struct FDBASkillEffectRow;

/**
 * UDBAEffectPlayer
 * 技能特效播放器
 * 统一管理所有技能反馈的播放
 */
UCLASS(Abstract, Blueprintable)
class DIVINEBEASTSARENA_API UDBAEffectPlayer : public USubsystem
{
	GENERATED_BODY()

public:
	UDBAEffectPlayer();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	/** 播放技能释放效果 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Feedback|Player")
	void PlayReleaseEffect(AActor* Caster, FName SkillID, FVector Location, FRotator Direction);

	/** 播放技能命中效果 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Feedback|Player")
	void PlayHitEffect(AActor* Target, FName SkillID, FVector ImpactPoint);

	/** 播放音效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Feedback|Player")
	void PlaySound(AActor* Target, FName SkillID, bool bIsHit);

	/** 触发屏幕震动 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Feedback|Player")
	void TriggerScreenShake(AActor* Target, FName SkillID, float Scale = 1.0f);

	/** 生成伤害数字 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Feedback|Player")
	void SpawnDamageNumber(AActor* Target, float Damage, bool bIsCritical, uint8 ElementValue, FVector ImpactPoint);

	/** 设置特效表管理器 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Feedback|Player")
	void SetEffectTableManager(UDBAEffectTableManager* Manager);

	/** 获取特效表管理器 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Feedback|Player")
	UDBAEffectTableManager* GetEffectTableManager() const { return EffectTableManager.Get(); }

	/** 获取单例 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Feedback|Player")
	static UDBAEffectPlayer* Get(UWorld* World);

protected:
	/** 获取技能特效数据 */
	const FDBASkillEffectRow* GetSkillEffectData(FName SkillID) const;

	/** 安全播放Niagara特效 */
	UNiagaraComponent* SafeSpawnNiagaraEffect(TSubclassOf<UNiagaraSystem> SystemClass, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

	/** 安全播放音效 */
	void SafePlaySound(USoundBase* SoundBase, FVector Location, bool bIs3D = true);

private:
	/** 特效表管理器 */
	UPROPERTY()
	TWeakObjectPtr<UDBAEffectTableManager> EffectTableManager;

	/** 世界引用 */
	UPROPERTY()
	TWeakObjectPtr<UWorld> CachedWorld;
};