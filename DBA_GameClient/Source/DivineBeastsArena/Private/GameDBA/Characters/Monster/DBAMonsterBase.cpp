// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// 怪物模型基类

#include "GameDBA/Characters/Monster/DBAMonsterBase.h"
#include "GameDBA/Presentation/VFX/Feedback/DBAFloatingDamageComponent.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameCore/Async/DBAAsyncAssetLoader.h"
#include "GameDBA/Gameplay/GAS/DBAAbilitySystemComponent.h"
#include "GameDBA/Gameplay/Progression/Attributes/DBABattleAttributeSet.h"
#include "GameDBA/Gameplay/Progression/Attributes/DBABattleAttributeDefaultsDataAsset.h"
#include "GameDBA/Gameplay/Progression/Attributes/DBABattleAttributeDeveloperSettings.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

ADBAMonsterBase::ADBAMonsterBase()
{
	// 基类 ADBACharacterBase 构造函数已设置 PrimaryActorTick、bReplicates、SetReplicateMovement
	// 此处仅保留怪物特有的碰撞配置
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
	GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));

	// P2-3 改造：创建 GAS 组件（AI 角色 ASC 持有在自身）
	AbilitySystemComponent = CreateDefaultSubobject<UDBAAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	// AI 角色使用 Mixed 复制模式：自身属性复制到所有人，GE 仅复制给 self
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	BattleAttributeSet = CreateDefaultSubobject<UDBABattleAttributeSet>(TEXT("BattleAttributeSet"));
}

// ==================== 基类虚函数重写 ====================

UAbilitySystemComponent* ADBAMonsterBase::GetAbilitySystemComponent() const
{
	// P2-3 改造：返回自身创建的 UDBAAbilitySystemComponent
	return AbilitySystemComponent;
}

float ADBAMonsterBase::GetCurrentHealth() const
{
	// P2-3 改造：从 GAS AttributeSet 读取权威值
	if (BattleAttributeSet)
	{
		return BattleAttributeSet->GetCurrentHealth();
	}
	return 0.0f;
}

float ADBAMonsterBase::GetMaxHealth() const
{
	// P2-3 改造：从 GAS AttributeSet 读取权威值
	if (BattleAttributeSet)
	{
		return BattleAttributeSet->GetMaxHealth();
	}
	return 0.0f;
}

void ADBAMonsterBase::BeginPlay()
{
	Super::BeginPlay();

	// P2-3 改造：初始化 ASC AbilityActorInfo（AI 角色 OwnerActor=AvatarActor=this）
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}

	// 异步加载默认战斗属性（参考 ADBAPlayerState 链路）
	RequestDefaultBattleAttributeDefaultsAsync();

	static const TSoftObjectPtr<UNiagaraSystem> HitVFX(FSoftObjectPath(TEXT("/Game/DBA/VFX/Common/Impact/NS_Impact_Generic_Hit.NS_Impact_Generic_Hit")));
	TArray<FSoftObjectPath> Paths;
	DBAAsyncAssetLoader::AddPreloadPath(HitVFX, Paths);
	DBAAsyncAssetLoader::RequestAsyncPreload(this, Paths);
}

// ==================== GAS 异步加载链路 ====================

void ADBAMonsterBase::RequestDefaultBattleAttributeDefaultsAsync()
{
	const UDBABattleAttributeDeveloperSettings* Settings = GetDefault<UDBABattleAttributeDeveloperSettings>();
	const TSoftObjectPtr<UDBABattleAttributeDefaultsDataAsset> DefaultsAsset = Settings
		? Settings->DefaultBattleAttributeDefaults
		: TSoftObjectPtr<UDBABattleAttributeDefaultsDataAsset>();
	if (DefaultsAsset.IsNull())
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[怪物] 未配置默认战斗属性数据资产：怪物=%s。请在 DBA 战斗属性设置中配置 DefaultBattleAttributeDefaults。"), *GetName());
		return;
	}

	// 已加载则直接应用
	if (UDBABattleAttributeDefaultsDataAsset* LoadedDefaults = DefaultsAsset.Get())
	{
		HandleDefaultBattleAttributeDefaultsLoaded(LoadedDefaults);
		return;
	}

	// 异步加载，使用弱引用保护生命周期
	TWeakObjectPtr<ADBAMonsterBase> WeakThis(this);
	DBAAsyncAssetLoader::RequestAsyncAsset<UDBABattleAttributeDefaultsDataAsset>(this, DefaultsAsset, [WeakThis](UDBABattleAttributeDefaultsDataAsset* LoadedDefaults)
	{
		if (ADBAMonsterBase* StrongThis = WeakThis.Get())
		{
			StrongThis->HandleDefaultBattleAttributeDefaultsLoaded(LoadedDefaults);
		}
	});
}

void ADBAMonsterBase::HandleDefaultBattleAttributeDefaultsLoaded(UDBABattleAttributeDefaultsDataAsset* LoadedDefaults)
{
	if (!LoadedDefaults)
	{
		UE_LOG(LogDBACombat, Error, TEXT("[怪物] 默认战斗属性数据资产异步加载失败：怪物=%s"), *GetName());
		return;
	}

	if (!BattleAttributeSet)
	{
		UE_LOG(LogDBACombat, Error, TEXT("[怪物] 应用默认战斗属性失败：BattleAttributeSet 不可用。怪物=%s"), *GetName());
		return;
	}

	BattleAttributeSet->ApplyDefaultAttributes(LoadedDefaults);

	UE_LOG(LogDBACombat, Log, TEXT("[怪物] 默认战斗属性已应用：怪物=%s 类型=%s 生命=%.1f/%.1f"),
		*GetName(), *MonsterType.ToString(),
		BattleAttributeSet->GetCurrentHealth(),
		BattleAttributeSet->GetMaxHealth());
}

// ==================== 战斗逻辑 ====================

float ADBAMonsterBase::GetHealthPercent() const
{
	const float MaxHP = GetMaxHealth();
	return MaxHP > 0.0f ? FMath::Clamp(GetCurrentHealth() / MaxHP, 0.0f, 1.0f) : 0.0f;
}

float ADBAMonsterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float AppliedDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	const float FinalDamage = AppliedDamage > 0.0f ? AppliedDamage : DamageAmount;

	if (!HasAuthority() || FinalDamage <= 0.0f || !BattleAttributeSet)
	{
		return FinalDamage;
	}

	// P2-3 改造：通过 AttributeSet 修改 CurrentHealth（权威路径）
	const float CurrentHP = BattleAttributeSet->GetCurrentHealth();
	if (CurrentHP <= 0.0f)
	{
		return FinalDamage;
	}

	const float NewHP = FMath::Clamp(CurrentHP - FinalDamage, 0.0f, BattleAttributeSet->GetMaxHealth());
	BattleAttributeSet->SetCurrentHealth(NewHP);

	PlayHitVFX(DamageCauser);
	MulticastShowDamageNumber(FinalDamage, GetActorLocation() + FVector(0.0f, 0.0f, 118.0f), false);

	UE_LOG(LogDBACombat, Log, TEXT("[怪物] 怪物受到伤害：怪物=%s 类型=%s 伤害=%.1f 生命=%.1f/%.1f 伤害来源=%s"),
		*GetName(),
		*MonsterType.ToString(),
		FinalDamage,
		NewHP,
		BattleAttributeSet->GetMaxHealth(),
		*GetNameSafe(DamageCauser));

	if (NewHP <= 0.0f)
	{
		HandleMonsterDefeated(DamageCauser);
	}

	return FinalDamage;
}

void ADBAMonsterBase::HandleMonsterDefeated(AActor* DamageCauser)
{
	PlayDeathVFX();
	UE_LOG(LogDBACombat, Log, TEXT("[怪物] 怪物已被击败：怪物=%s 类型=%s 伤害来源=%s"),
		*GetName(),
		*MonsterType.ToString(),
		*GetNameSafe(DamageCauser));
	SetLifeSpan(1.0f);
	SetActorEnableCollision(false);
	if (GetMesh())
	{
		GetMesh()->SetHiddenInGame(true);
	}
}

void ADBAMonsterBase::MulticastShowDamageNumber_Implementation(float DamageAmount, FVector_NetQuantize ImpactPoint, bool bIsCritical)
{
	if (UDBAFloatingDamageComponent* DamageComponent = FindComponentByClass<UDBAFloatingDamageComponent>())
	{
		DamageComponent->SpawnDamageNumber(DamageAmount, bIsCritical, 4, FVector(ImpactPoint));
	}
}

void ADBAMonsterBase::PlayHitVFX(AActor* Attacker)
{
	if (!GetWorld())
	{
		return;
	}

	FVector HitLocation = GetActorLocation();

	// P1-6 改造：使用 TSoftObjectPtr 软引用替代同步加载
	static const TSoftObjectPtr<UNiagaraSystem> HitVFXRef(FSoftObjectPath(TEXT("/Game/DBA/VFX/Common/Impact/NS_Impact_Generic_Hit.NS_Impact_Generic_Hit")));
	if (UNiagaraSystem* VFX = HitVFXRef.Get())
	{
		FRotator Rotation = FRotator::ZeroRotator;
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), VFX, HitLocation, Rotation, FVector(1.0f), true, true);
	}
}

void ADBAMonsterBase::PlayDeathVFX()
{
	// 死亡特效由后续怪物表现数据资产提供；当前不引用已移除的第三方资源。
}

void ADBAMonsterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// P2-3 改造：CurrentHealth 已由 BattleAttributeSet 内部注册 DOREPLIFETIME，此处不再注册避免冲突
}
