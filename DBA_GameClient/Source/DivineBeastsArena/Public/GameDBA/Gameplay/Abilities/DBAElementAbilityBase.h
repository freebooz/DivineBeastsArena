// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameDBA/Gameplay/Abilities/DBAZodiacAbilityBase.h"
#include "GameCore/Types/DBACommonEnums.h"
#include "DBAElementAbilityBase.generated.h"

class UGameplayEffect;
struct FDBAAbilityRuntimeConfig;

/**
 * 自然元素之力技能基类 (Active Skill01~04)
 * 决定克制、属性加成、技能元素
 */
UCLASS(Abstract)
class DIVINEBEASTSARENA_API UDBAElementAbilityBase : public UDBAMobaGameplayAbilityBase
{
	GENERATED_BODY()

public:
	UDBAElementAbilityBase();

	/**
	 * 获取技能所属的自然元素类型
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Element")
	EDBAElement GetElementType() const { return AbilityElementType; }

	/** 所属自然元素之力 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Element")
	EDBAElement AbilityElementType = EDBAElement::None;

	/** 是否消耗 CurrentEnergy */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Element")
	float AbilityEnergyCost;

	/** 冷却标签；未配置时按 GAS 输入槽自动映射到 Cooldown.Skill01~Skill04。 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DBA|Element|Cooldown", meta = (Categories = "Cooldown"))
	FGameplayTag CooldownTag;

protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual bool CommitAbilityCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) override;

	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

	virtual bool CheckCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

	virtual void GetCooldownTimeRemainingAndDuration(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, float& TimeRemaining, float& CooldownDuration) const override;

	const FDBAAbilityRuntimeConfig* ResolveRuntimeConfig(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const;

	float ResolveRuntimeEnergyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const;

	float ResolveRuntimeCooldownDuration(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const;

	TSubclassOf<UGameplayEffect> ResolveRuntimeCostGameplayEffectClass(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const;

	TSubclassOf<UGameplayEffect> ResolveRuntimeCooldownGameplayEffectClass(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const;

	FGameplayTag ResolveCooldownTag(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const;
};
