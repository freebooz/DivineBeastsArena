// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// 技能VFX/SFX管理器

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DBASkillVFXManager.generated.h"

class UParticleSystem;
class USoundBase;
class UAnimMontage;

/**
 * UDBASkillVFXManager
 * 技能VFX/SFX管理器
 * 统一管理所有技能的视觉和音效资源
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBASkillVFXManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UDBASkillVFXManager();

public:
	// ==================== 技能特效播放 ====================

	/** 播放技能特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillVFX")
	void PlaySkillVFX(FName SkillId, AActor* Target, AActor* Owner);

	/** 停止技能特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillVFX")
	void StopSkillVFX(FName SkillId);

	/** 播放技能音效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillSFX")
	void PlaySkillSFX(FName SkillId, AActor* Owner);

	/** 停止技能音效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|SkillSFX")
	void StopSkillSFX(FName SkillId);

public:
	// ==================== 资源加载 ====================

	/** 预加载所有技能资源 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Resources")
	void PreloadAllSkillResources();

	/** 卸载所有技能资源 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Resources")
	void UnloadAllSkillResources();

protected:
	// ==================== 资源路径映射 ====================

	/** 获取技能VFX路径 */
	UFUNCTION(BlueprintPure, Category = "DBA|Resources")
	FString GetSkillVFXPath(FName SkillId);

	/** 获取技能SFX路径 */
	UFUNCTION(BlueprintPure, Category = "DBA|Resources")
	FString GetSkillSFXPath(FName SkillId);

protected:
	// ==================== 资源数据 ====================

	/** 技能施法特效映射 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Resources")
	TMap<FName, TSoftObjectPtr<UParticleSystem>> SkillCastingVFX;

	/** 技能命中特效映射 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Resources")
	TMap<FName, TSoftObjectPtr<UParticleSystem>> SkillImpactVFX;

	/** 技能施法音效映射 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Resources")
	TMap<FName, TSoftObjectPtr<USoundBase>> SkillCastingSFX;

	/** 技能命中音效映射 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Resources")
	TMap<FName, TSoftObjectPtr<USoundBase>> SkillImpactSFX;
};