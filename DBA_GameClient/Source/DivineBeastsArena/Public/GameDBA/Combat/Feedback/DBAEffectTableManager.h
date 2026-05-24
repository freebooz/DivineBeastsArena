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
#include "Subsystems/GameInstanceSubsystem.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "Camera/CameraShakeBase.h"
#include "DBAEffectTableManager.generated.h"

class UCameraShakeBase;

/**
 * FDBASkillEffectRow
 * 技能特效表数据结构
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBASkillEffectRow
{
	GENERATED_BODY()

	/** 技能唯一标识 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName SkillID;

	/** 技能显示名称 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText SkillName;

	/** 释放特效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UNiagaraSystem> ReleaseEffect;

	/** 命中特效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UNiagaraSystem> HitEffect;

	/** 释放音效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<USoundBase> CastSound;

	/** 命中音效 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<USoundBase> HitSound;

	/** 屏幕震动类 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UCameraShakeBase> ScreenShakeClass;

	/** 震动强度系数 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (UIMin = 0.0, UIMax = 2.0))
	float ShakeScale = 1.0f;

	/** 伤害数字颜色 (普通) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FLinearColor DamageNumberColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

	/** 暴击颜色 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FLinearColor CriticalColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);

	/** 加载状态 */
	bool IsLoaded() const
	{
		return ReleaseEffect.IsValid() || HitEffect.IsValid() || CastSound.IsValid() || HitSound.IsValid();
	}
};

/**
 * UDBAEffectTableManager
 * 技能特效表管理器
 * 负责加载和管理技能特效数据表
 */
UCLASS(Abstract, Blueprintable)
class DIVINEBEASTSARENA_API UDBAEffectTableManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UDBAEffectTableManager();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	/** 兼容旧蓝图入口：异步预加载技能特效表和表内表现资源 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Effect|Table")
	void LoadSkillEffectTable(const TSoftObjectPtr<UDataTable>& TablePath);

	/** 异步加载技能特效表 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Effect|Table")
	TSoftObjectPtr<UDataTable> AsyncLoadSkillEffectTable(const TSoftObjectPtr<UDataTable>& TablePath);

	/** 查询技能特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Effect|Table")
	FDBASkillEffectRow GetSkillEffect(FName SkillID) const;

	/** 获取所有技能ID */
	UFUNCTION(BlueprintCallable, Category = "DBA|Effect|Table")
	TArray<FName> GetAllSkillIDs() const;

	/** 检查是否已加载 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Effect|Table")
	bool IsTableLoaded() const { return SkillEffectTable != nullptr; }

protected:
	/** 异步加载完成回调 */
	void OnAsyncLoadComplete(TSoftObjectPtr<UDataTable> TablePath);

private:
	/** 技能特效表 */
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> SkillEffectTable;

	/** 已加载的特效数据缓存 */
	UPROPERTY(Transient)
	TMap<FName, FDBASkillEffectRow> CachedEffects;

	/** 待加载路径 */
	UPROPERTY(Transient)
	TSoftObjectPtr<UDataTable> PendingTablePath;
};
