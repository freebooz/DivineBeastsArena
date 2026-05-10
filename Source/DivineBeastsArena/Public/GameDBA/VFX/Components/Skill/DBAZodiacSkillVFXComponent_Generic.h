// Copyright Freebooz Games, Inc. All Rights Reserved.
// 泛化技能VFX组件 - 通过ZodiacType + SkillSlot从DataTable配置

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameCore/Types/DBACommonEnums.h"
#include "GameDBA/VFX/Structs/DBAVFXDataRow.h"
#include "DBAZodiacSkillVFXComponent_Generic.generated.h"

class UParticleSystem;
class USoundBase;
class UAnimMontage;
class UDataTable;

/**
 * UDBAZodiacSkillVFXComponent_Generic
 * 泛化技能VFX/SFX挂载组件
 * 通过 ZodiacType + SkillSlot 从 DataTable 加载配置
 * 替代原有的 60 个具体生肖技能VFX组件
 */
UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = "DBA Skill VFX Generic"))
class DIVINEBEASTSARENA_API UDBAZodiacSkillVFXComponent_Generic : public UActorComponent
{
	GENERATED_BODY()

public:
	UDBAZodiacSkillVFXComponent_Generic();

public:
	// ==================== 配置接口 ====================

	/** 生肖类型 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	EDBAZodiac ZodiacType = EDBAZodiac::None;

	/** 技能槽位名称 (Q/W/E/R/Passive) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	FName SkillSlot = NAME_None;

	/** VFX DataTable 引用 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	TObjectPtr<UDataTable> VFXDataTable;

public:
	/** 从DataTable加载VFX配置 */
	UFUNCTION(BlueprintCallable, Category = "DBA|VFX")
	void LoadFromDataTable();

	/** 获取当前VFX数据行 */
	UFUNCTION(BlueprintCallable, Category = "DBA|VFX")
	FDBAVFXDataRow* GetVFXData() const;

	// ==================== 技能特效接口 ====================

	/** 播放施法特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillVFX")
	void PlayCastingVFX(AActor* Target);

	/** 播放命中特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillVFX")
	void PlayImpactVFX(AActor* HitTarget);

	/** 播放飞行特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillVFX")
	void PlayProjectileVFX(FVector Start, FVector End);

	/** 播放范围特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillVFX")
	void PlayAOEVFX(FVector Center, float Radius);

	/** 播放引导特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillVFX")
	void PlayChannelVFX();

	/** 停止引导特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillVFX")
	void StopChannelVFX();

	// ==================== 技能音效接口 ====================

	/** 播放施法音效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillSFX")
	void PlayCastingSFX();

	/** 播放飞行音效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillSFX")
	void PlayProjectileSFX();

	/** 播放命中音效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillSFX")
	void PlayImpactSFX();

protected:
	/** 缓存的VFX数据行 */
	UPROPERTY()
	FDBAVFXDataRow CachedVFXData;

	/** 引导特效的粒子组件 */
	UPROPERTY()
	TObjectPtr<UParticleSystemComponent> ChannelVFXComponent;
};