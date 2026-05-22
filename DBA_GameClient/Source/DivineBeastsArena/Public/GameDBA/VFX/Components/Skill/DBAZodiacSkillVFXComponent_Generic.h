// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// 泛化技能VFX组件 - 通过ZodiacType + SkillSlot从DataTable配置

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Damage")
	bool bApplyDamageOnImpact = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Damage", meta = (ClampMin = "0.0"))
	float BaseDamage = 50.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Damage")
	EDBAElement AttackElement = EDBAElement::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Damage")
	EDBAElement FallbackDefenseElement = EDBAElement::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Damage", meta = (ClampMin = "-1", ClampMax = "4"))
	int32 ResonanceLevelOverride = -1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Damage", meta = (ClampMin = "-1", ClampMax = "10"))
	int32 ChainLevelOverride = -1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Damage", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float CriticalRateOverride = -1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Damage", meta = (ClampMin = "1.0"))
	float CriticalMultiplier = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Damage", meta = (ClampMin = "0.0"))
	float DefaultAOEDamageRadius = 250.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Cue")
	FGameplayTag CastingCueTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Cue")
	FGameplayTag ProjectileCueTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Cue")
	FGameplayTag ImpactCueTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Cue")
	FGameplayTag AOECueTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Cue")
	FGameplayTag ChannelCueTag;

public:
	/** 从DataTable加载VFX配置 */
	UFUNCTION(BlueprintCallable, Category = "DBA|VFX")
	void LoadFromDataTable();

	/** 获取当前VFX数据行 */
	UFUNCTION(BlueprintCallable, Category = "DBA|VFX")
	const FDBAVFXDataRow& GetVFXData() const;

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

	UFUNCTION(BlueprintCallable, Category = "DBA|SkillDamage")
	float CalculateSkillDamage(AActor* HitTarget, bool& bOutIsCritical) const;

	UFUNCTION(BlueprintCallable, Category = "DBA|SkillDamage")
	bool ApplySkillDamage(AActor* HitTarget, FVector HitLocation, float& OutFinalDamage, bool& bOutIsCritical);

	UFUNCTION(BlueprintCallable, Category = "DBA|SkillDamage")
	int32 ApplyAOEDamage(FVector Center, float Radius, UPARAM(ref) TArray<AActor*>& OutHitActors);

	UFUNCTION(BlueprintCallable, Category = "DBA|SkillCue")
	void ExecuteSkillGameplayCue(const FGameplayTag& CueTag, AActor* CueTarget, FVector Location, float RawMagnitude, bool bIsCritical) const;

protected:
	/** 缓存的VFX数据行 */
	UPROPERTY(Transient)
	FDBAVFXDataRow CachedVFXData;

	/** 引导特效的粒子组件 */
	UPROPERTY(Transient)
	TObjectPtr<UParticleSystemComponent> ChannelVFXComponent;
};
