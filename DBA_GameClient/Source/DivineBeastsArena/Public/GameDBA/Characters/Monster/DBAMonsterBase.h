// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// 怪物模型基类

#pragma once

#include "CoreMinimal.h"
#include "GameDBA/Characters/DBACharacterBase.h"
#include "DBAMonsterBase.generated.h"

class UDBAAbilitySystemComponent;
class UDBABattleAttributeSet;
class UDBABattleAttributeDefaultsDataAsset;

/**
 * ADBAMonsterBase
 * 怪物统一基类
 *
 * 继承 ADBACharacterBase（统一角色基类），复用死亡状态、队伍 ID、IAbilitySystemInterface 接入点等公共能力。
 *
 * P2-3 改造（2026-07-07）：
 *   - 已接入 GAS：自身持有 UDBAAbilitySystemComponent + UDBABattleAttributeSet
 *   - 生命值由 BattleAttributeSet 的 MaxHealth/CurrentHealth 属性驱动
 *   - 通过 DeveloperSettings 异步加载默认战斗属性（参考 ADBAPlayerState 链路）
 *   - TakeDamage 内部走 AttributeSet 路径修改 CurrentHealth
 *   - 旧的 MaxHealth/CurrentHealth 字段已废弃，仅保留蓝图兼容性
 *
 * 设计依据：
 *   - 项目策略《全局 C++ 逻辑实现策略》：所有逻辑相关实现使用 C++
 *   - 项目策略《DBA.DataAsset.NoHardcoding》：怪物配置通过 DataAsset 驱动
 *   - 项目策略《DBA.UI.EventAsync》：异步加载默认属性
 *   - 用户要求："所有角色和怪物均要设计基础类，从继承类继承"
 */
UCLASS(Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API ADBAMonsterBase : public ADBACharacterBase
{
	GENERATED_BODY()

public:
	ADBAMonsterBase();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ==================== 基类虚函数重写 ====================

	/**
	 * 获取能力系统组件
	 * P2-3 改造：返回自身创建的 UDBAAbilitySystemComponent（AI 角色 ASC 持有在自身）
	 */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/** 获取当前生命值（重写基类，从 GAS AttributeSet 读取） */
	virtual float GetCurrentHealth() const override;

	/** 获取最大生命值（重写基类，从 GAS AttributeSet 读取） */
	virtual float GetMaxHealth() const override;

protected:
	virtual void BeginPlay() override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void HandleMonsterDefeated(AActor* DamageCauser);

public:
	/** 播放受击特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Monster")
	void PlayHitVFX(AActor* Attacker);

	/** 播放死亡特效 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Monster")
	void PlayDeathVFX();

	UFUNCTION(BlueprintCallable, Category = "DBA|Monster")
	float GetHealthPercent() const;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastShowDamageNumber(float DamageAmount, FVector_NetQuantize ImpactPoint, bool bIsCritical);

protected:
	// ==================== GAS 异步加载链路（参考 ADBAPlayerState） ====================

	/** 从 DeveloperSettings 异步加载默认战斗属性数据资产 */
	void RequestDefaultBattleAttributeDefaultsAsync();

	/** 默认战斗属性加载完成回调 */
	void HandleDefaultBattleAttributeDefaultsLoaded(UDBABattleAttributeDefaultsDataAsset* LoadedDefaults);

protected:
	/** 怪物类型标识（用于日志、配置查找） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Config")
	FName MonsterType = FName(TEXT("None"));

	/** 怪物等级 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Config")
	int32 Level = 1;

	// ==================== GAS 组件（P2-3 新增） ====================

	/** 怪物能力系统组件（AI 角色持有在自身） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DBA|Monster|GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDBAAbilitySystemComponent> AbilitySystemComponent;

	/** 战斗属性集（MaxHealth/CurrentHealth 等权威属性来源） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DBA|Monster|GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDBABattleAttributeSet> BattleAttributeSet;

	// ==================== 已废弃字段（保留蓝图兼容性） ====================

	/**
	 * 最大生命值（已废弃）
	 * @deprecated P2-3 改造：权威值由 BattleAttributeSet->MaxHealth 提供，请使用 GetMaxHealth() 读取。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|Config", meta = (DeprecatedProperty))
	float MaxHealth = 100.0f;

	/**
	 * 当前生命值（已废弃，原复制字段）
	 * @deprecated P2-3 改造：权威值由 BattleAttributeSet->CurrentHealth 提供，请使用 GetCurrentHealth() 读取。
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DBA|State", meta = (DeprecatedProperty))
	float CurrentHealth = 100.0f;
};
