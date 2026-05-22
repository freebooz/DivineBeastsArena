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
#include "Subsystems/WorldSubsystem.h"
#include "DBAEffectPlayer.generated.h"

class UDBAEffectTableManager;
class UDBAFloatingDamageComponent;
class UNiagaraComponent;
class UNiagaraSystem;
struct FDBASkillEffectRow;

/**
 * UDBAEffectPlayer
 * 技能特效播放器
 * 统一管理所有技能反馈的播放
 */
UCLASS(Abstract, Blueprintable)
class DIVINEBEASTSARENA_API UDBAEffectPlayer : public UWorldSubsystem
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
	bool GetSkillEffectData(FName SkillID, FDBASkillEffectRow& OutEffectData) const;

	/** 安全播放Niagara特效 */
	UNiagaraComponent* SafeSpawnNiagaraEffect(UNiagaraSystem* System, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

	/** 安全播放音效 */
	void SafePlaySound(USoundBase* SoundBase, FVector Location, bool bIs3D = true);

private:
	/** 特效表管理器 */
	UPROPERTY(Transient)
	TWeakObjectPtr<UDBAEffectTableManager> EffectTableManager;

	/** 世界引用 */
	UPROPERTY(Transient)
	TWeakObjectPtr<UWorld> CachedWorld;
};
