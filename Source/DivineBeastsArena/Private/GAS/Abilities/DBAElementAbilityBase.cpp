// Copyright FreeboozStudio. All Rights Reserved.

#include "GAS/Abilities/DBAElementAbilityBase.h"
#include "GAS/DBAAbilitySystemComponent.h"
#include "GAS/Attributes/DBABattleAttributeSet.h"
#include "GAS/Effects/DBEEnergyCostEffect.h"
#include "Common/DBALogChannels.h"

UDBAElementAbilityBase::UDBAElementAbilityBase()
{
	ElementType = EDBAElement::Gold;
	EnergyCost = 0.0f;
}

bool UDBAElementAbilityBase::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// 防御性检查：确保 ActorInfo 有效
	ensure(ActorInfo != nullptr);
	ensure(ActorInfo->AbilitySystemComponent.IsValid());

	// 校验 CurrentEnergy 是否足够
	if (EnergyCost > 0.0f && ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
		if (!ASC)
		{
			return false;
		}

		// 通过 AttributeAccessor 获取 CurrentEnergy
		const UDBABattleAttributeSet* CombatAttrSet = ASC->GetSet<UDBABattleAttributeSet>();
		if (!CombatAttrSet)
		{
			UE_LOG(LogDBACombat, Warning, TEXT("[DBAElementAbilityBase] 无法获取 DBABattleAttributeSet"));
			return false;
		}

		float CurrentEnergy = CombatAttrSet->GetCurrentEnergy();
		float MaxEnergy = CombatAttrSet->GetMaxEnergy();

		if (CurrentEnergy < EnergyCost)
		{
			UE_LOG(LogDBACombat, Warning, TEXT("[DBAElementAbilityBase] 能量不足：需要 %.1f，当前 %.1f / %.1f"), EnergyCost, CurrentEnergy, MaxEnergy);
			return false;
		}
	}

	return true;
}

bool UDBAElementAbilityBase::CommitAbilityCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, OUT FGameplayTagContainer* OptionalRelevantTags)
{
	// 防御性检查
	ensure(ActorInfo != nullptr);
	if (ActorInfo)
	{
		ensure(ActorInfo->AbilitySystemComponent.IsValid());
	}

	// 先校验能量
	if (EnergyCost > 0.0f && ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
		if (!ASC)
		{
			return false;
		}

		const UDBABattleAttributeSet* CombatAttrSet = ASC->GetSet<UDBABattleAttributeSet>();
		if (!CombatAttrSet)
		{
			return false;
		}

		float CurrentEnergy = CombatAttrSet->GetCurrentEnergy();

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

	bool bCostCommited = Super::CommitAbilityCost(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags);

	return bCostCommited;
}
