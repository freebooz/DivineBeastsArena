// Copyright FreeboozStudio. All Rights Reserved.
// 元素能力基类实现 - 所有元素相关能力的基类

#include "DBA/GAS/Abilities/DBAElementAbilityBase.h"
#include "DBA/GAS/DBAAbilitySystemComponent.h"
#include "DBA/GAS/Attributes/DBABattleAttributeSet.h"
#include "DBA/GAS/Effects/DBEEnergyCostEffect.h"
#include "Common/DBALogChannels.h"

// 构造函数 - 初始化元素能力默认属性
UDBAElementAbilityBase::UDBAElementAbilityBase()
{
    // 默认元素类型为金
	ElementType = EDBAElement::Gold;
	// 默认能量消耗为0
	EnergyCost = 0.0f;
}

// CanActivateAbility - 检查能力是否可以激活
bool UDBAElementAbilityBase::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
    // 调用父类检查
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// 防御性检查：确保 ActorInfo 有效
	ensure(ActorInfo != nullptr);
	ensure(ActorInfo->AbilitySystemComponent.IsValid());

	// 校验能量是否足够
	if (EnergyCost > 0.0f && ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
        // 获取能力系统组件
		UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
		if (!ASC)
		{
			return false;
		}

		// 获取战斗属性集
		const UDBABattleAttributeSet* CombatAttrSet = ASC->GetSet<UDBABattleAttributeSet>();
		if (!CombatAttrSet)
		{
			UE_LOG(LogDBACombat, Warning, TEXT("[DBAElementAbilityBase] 无法获取 DBABattleAttributeSet"));
			return false;
		}

		// 获取当前能量和最大能量
		float CurrentEnergy = CombatAttrSet->GetCurrentEnergy();
		float MaxEnergy = CombatAttrSet->GetMaxEnergy();

		// 检查能量是否足够
		if (CurrentEnergy < EnergyCost)
		{
			UE_LOG(LogDBACombat, Warning, TEXT("[DBAElementAbilityBase] 能量不足：需要 %.1f，当前 %.1f / %.1f"), EnergyCost, CurrentEnergy, MaxEnergy);
			return false;
		}
	}

	return true;
}

// CommitAbilityCost - 提交能力消耗（能量消耗）
bool UDBAElementAbilityBase::CommitAbilityCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags)
{
	// 防御性检查
	ensure(ActorInfo != nullptr);
	if (ActorInfo)
	{
		ensure(ActorInfo->AbilitySystemComponent.IsValid());
	}

	// 先校验能量是否足够
	if (EnergyCost > 0.0f && ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
		if (!ASC)
		{
			return false;
		}

		// 获取战斗属性集
		const UDBABattleAttributeSet* CombatAttrSet = ASC->GetSet<UDBABattleAttributeSet>();
		if (!CombatAttrSet)
		{
			return false;
		}

		// 获取当前能量
		float CurrentEnergy = CombatAttrSet->GetCurrentEnergy();

		// 再次检查能量是否足够
		if (CurrentEnergy < EnergyCost)
		{
			UE_LOG(LogDBACombat, Warning, TEXT("[DBAElementAbilityBase] 能量不足：需要 %.1f，当前 %.1f"), EnergyCost, CurrentEnergy);
			return false;
		}

		// 通过 GameplayEffect 应用能量消耗，确保预测和复制系统正常工作
		// 创建能量消耗 GameplayEffect Spec 并应用
		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		EffectContext.AddSourceObject(ActorInfo->AvatarActor.Get());

		// 防御性检查：确保 EffectContext 和 AvatarActor 有效
		ensure(EffectContext.IsValid());
		ensure(ActorInfo->AvatarActor.IsValid());

		// 创建能量消耗 GE Spec（使用内置的 CostGameplayEffect 机制）
		TSubclassOf<UGameplayEffect> EnergyCostEffectClass = UDBEEnergyCostEffect::StaticClass();
		FGameplayEffectSpec EffectSpec(EnergyCostEffectClass.GetDefaultObject(), EffectContext, EnergyCost);

		// 应用到自身
		FActiveGameplayEffectHandle ActiveHandle = ASC->ApplyGameplayEffectSpecToSelf(EffectSpec);

		// 检查能量消耗是否成功
		if (ActiveHandle.IsValid())
		{
			UE_LOG(LogDBACombat, Log, TEXT("[DBAElementAbilityBase] 能量消耗成功：%.1f"), EnergyCost);
		}
		else
		{
			UE_LOG(LogDBACombat, Warning, TEXT("[DBAElementAbilityBase] 能量消耗失败"));
			return false;
		}

		return true;
	}

	// 如果没有能量消耗，则调用父类处理
	bool bCostCommited = Super::CommitAbilityCost(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags);

	return bCostCommited;
}