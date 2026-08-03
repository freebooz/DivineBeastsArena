// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// 泛化投射物类 - 通过SkillID + DataTable配置

#pragma once

#include "CoreMinimal.h"
#include "GameDBA/Gameplay/Abilities/Projectiles/DBASkillProjectileBase.h"
#include "Engine/DataTable.h"
#include "DBAProjectile_Generic.generated.h"

/**
 * FDBAProjectileDataRow
 * 投射物数据行
 */
USTRUCT(BlueprintType)
struct DIVINEBEASTSARENA_API FDBAProjectileDataRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 投射物ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	FName ProjectileId;

	/** 速度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float Speed = 1000.0f;

	/** 半径 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float Radius = 50.0f;

	/** 伤害 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float Damage = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	EDBAElement DamageElement = EDBAElement::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cue")
	FGameplayTag ProjectileCueTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cue")
	FGameplayTag ImpactCueTag;

	/** 飞行特效路径 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TSoftObjectPtr<UParticleSystem> ProjectileVFX;

	/** 命中特效路径 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TSoftObjectPtr<UParticleSystem> ImpactVFX;

	/** 飞行音效路径 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX")
	TSoftObjectPtr<USoundBase> FlySFX;

	/** 命中音效路径 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX")
	TSoftObjectPtr<USoundBase> ImpactSFX;
};

/**
 * ADBAProjectile_Generic
 * 泛化投射物类
 * 通过 SkillID 从 DataTable 读取配置
 * 替代原有的 ADBAProjectile_<Zodiac>_<Skill> 24个类
 *
 * 使用方式:
 * 1. 在蓝图中设置 SkillID 和 ProjectileTable
 * 2. 运行时通过 InitializeProjectile 加载配置
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBAProjectile_Generic : public ADBASkillProjectileBase
{
	GENERATED_BODY()

public:
	ADBAProjectile_Generic();

public:
	/** 投射物 DataTable 引用 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Config")
	TObjectPtr<UDataTable> ProjectileTable;

public:
	/** 初始化投射物 - 使用DataTable配置 */
	virtual void InitializeProjectile(
		FName InSkillId,
		AActor* InOwner,
		AActor* InTarget,
		float InDamage,
		float InSpeed,
		float InRadius) override;

	/** 从DataTable加载投射物配置 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Projectile")
	void LoadFromDataTable(FName InSkillId);
};
