// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// 泛化投射物类实现

#include "GameDBA/Combat/DBAProjectile_Generic.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "Components/SphereComponent.h"

ADBAProjectile_Generic::ADBAProjectile_Generic()
{
	// 默认值
	Speed = 1000.0f;
	Radius = 50.0f;
	Damage = 50.0f;
}

void ADBAProjectile_Generic::InitializeProjectile(
	FName InSkillId,
	AActor* InOwner,
	AActor* InTarget,
	float InDamage,
	float InSpeed,
	float InRadius)
{
	// 如果有DataTable，先尝试从DataTable加载
	if (ProjectileTable && !InSkillId.IsNone())
	{
		LoadFromDataTable(InSkillId);
	}

	const float EffectiveDamage = InDamage > 0.0f ? InDamage : Damage;
	const float EffectiveSpeed = InSpeed > 0.0f ? InSpeed : Speed;
	const float EffectiveRadius = InRadius > 0.0f ? InRadius : Radius;
	Super::InitializeProjectile(InSkillId, InOwner, InTarget, EffectiveDamage, EffectiveSpeed, EffectiveRadius);

	UE_LOG(LogDBACombat, Log, TEXT("[DBAProjectile_Generic] 初始化投射物：技能=%s 速度=%.1f 半径=%.1f 伤害=%.1f"),
		*InSkillId.ToString(), Speed, Radius, Damage);
}

void ADBAProjectile_Generic::LoadFromDataTable(FName InSkillId)
{
	if (!ProjectileTable)
	{
		return;
	}

	static const FString ContextString = TEXT("DBAProjectile_Generic");
	FDBAProjectileDataRow* Row = ProjectileTable->FindRow<FDBAProjectileDataRow>(InSkillId, ContextString, false);

	if (!Row)
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[DBAProjectile_Generic] 未找到投射物配置: %s"), *InSkillId.ToString());
		return;
	}

	// 应用配置
	Speed = Row->Speed;
	Radius = Row->Radius;
	Damage = Row->Damage;
	DamageElement = Row->DamageElement;
	ProjectileCueTag = Row->ProjectileCueTag;
	ImpactCueTag = Row->ImpactCueTag;
	ProjectileVFXAsset = Row->ProjectileVFX;
	ImpactVFXAsset = Row->ImpactVFX;
	FlySFXAsset = Row->FlySFX;
	ImpactSFXAsset = Row->ImpactSFX;

	UE_LOG(LogDBACombat, Log, TEXT("[DBAProjectile_Generic] 已从数据表加载：技能=%s 速度=%.1f"), *InSkillId.ToString(), Speed);
}
