// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Gameplay/Abilities/DBAElementAbilityBase.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameDBA/Core/DBAEnumsCore.h"
#include "GameDBA/Core/DBAGameplayTags.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/Gameplay/Progression/Attributes/DBABattleAttributeSet.h"
#include "GameDBA/Gameplay/GAS/DBAAbilitySystemComponent.h"
#include "GameDBA/Gameplay/GAS/Effects/DBAGE_Cooldown.h"

UDBAElementAbilityBase::UDBAElementAbilityBase()
{
	AbilityElementType = EDBAElement::None;
	AbilityEnergyCost = 0.0f;
	EnergyCost = 0.0f;
}

bool UDBAElementAbilityBase::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}
	const UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	const UDBABattleAttributeSet* AttrSet = ASC ? ASC->GetSet<UDBABattleAttributeSet>() : nullptr;
	return AttrSet && AttrSet->GetCurrentEnergy() >= ResolveRuntimeEnergyCost(Handle, ActorInfo);
}

bool UDBAElementAbilityBase::CommitAbilityCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FGameplayTagContainer* OptionalRelevantTags)
{
	return Super::CommitAbilityCost(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags);
}

bool UDBAElementAbilityBase::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags))
	{
		return false;
	}

	if (ResolveRuntimeCostGameplayEffectClass(Handle, ActorInfo))
	{
		return true;
	}

	const float Cost = ResolveRuntimeEnergyCost(Handle, ActorInfo);
	if (Cost <= 0.0f)
	{
		return true;
	}

	const UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	const UDBABattleAttributeSet* AttrSet = ASC ? ASC->GetSet<UDBABattleAttributeSet>() : nullptr;
	if (!AttrSet)
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[元素技能] 能量检查失败：缺少战斗属性集。"));
		return false;
	}

	const float CurrentEnergy = AttrSet->GetCurrentEnergy();
	if (CurrentEnergy + KINDA_SMALL_NUMBER < Cost)
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[元素技能] 能量检查失败：当前能量 %.2f 小于消耗 %.2f。"), CurrentEnergy, Cost);
		return false;
	}

	return true;
}

void UDBAElementAbilityBase::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	const TSubclassOf<UGameplayEffect> ResolvedCostGameplayEffectClass = ResolveRuntimeCostGameplayEffectClass(Handle, ActorInfo);
	if (ResolvedCostGameplayEffectClass)
	{
		if (GetCostGameplayEffect() && ResolvedCostGameplayEffectClass.Get() == GetCostGameplayEffect()->GetClass())
		{
			Super::ApplyCost(Handle, ActorInfo, ActivationInfo);
			return;
		}

		FGameplayEffectSpecHandle CostSpecHandle = MakeOutgoingGameplayEffectSpec(
			Handle,
			ActorInfo,
			ActivationInfo,
			ResolvedCostGameplayEffectClass,
			GetAbilityLevel(Handle, ActorInfo));
		if (!CostSpecHandle.IsValid() || !CostSpecHandle.Data.IsValid())
		{
			UE_LOG(LogDBACombat, Warning, TEXT("[元素技能] 提交能量消耗失败：无法创建配置的 Cost GameplayEffect。"));
			return;
		}

		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, CostSpecHandle);
		return;
	}

	const float Cost = ResolveRuntimeEnergyCost(Handle, ActorInfo);
	if (Cost <= 0.0f)
	{
		return;
	}

	UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	UDBABattleAttributeSet* AttrSet = ASC ? const_cast<UDBABattleAttributeSet*>(ASC->GetSet<UDBABattleAttributeSet>()) : nullptr;
	if (!AttrSet)
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[元素技能] 提交能量消耗失败：缺少战斗属性集。"));
		return;
	}

	if (!ActorInfo->IsNetAuthority())
	{
		return;
	}

	const float CurrentEnergy = AttrSet->GetCurrentEnergy();
	if (CurrentEnergy + KINDA_SMALL_NUMBER < Cost)
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[元素技能] 提交能量消耗失败：当前能量 %.2f 小于消耗 %.2f。"), CurrentEnergy, Cost);
		return;
	}

	const float NewEnergy = FMath::Clamp(CurrentEnergy - Cost, 0.0f, AttrSet->GetMaxEnergy());
	AttrSet->SetCurrentEnergy(NewEnergy);
	UE_LOG(LogDBACombat, Verbose, TEXT("[元素技能] 已提交能量消耗：消耗 %.2f，剩余 %.2f。"), Cost, NewEnergy);
}

bool UDBAElementAbilityBase::CheckCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CheckCooldown(Handle, ActorInfo, OptionalRelevantTags))
	{
		return false;
	}

	const float RuntimeCooldownDuration = ResolveRuntimeCooldownDuration(Handle, ActorInfo);
	if (RuntimeCooldownDuration <= 0.0f)
	{
		return true;
	}

	UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	const FGameplayTag ResolvedCooldownTag = ResolveCooldownTag(Handle, ActorInfo);
	if (!ASC || !ResolvedCooldownTag.IsValid())
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[元素技能] 冷却检查失败：缺少能力系统组件或冷却标签。"));
		return false;
	}

	FGameplayTagContainer CooldownTags;
	CooldownTags.AddTag(ResolvedCooldownTag);
	const FGameplayEffectQuery CooldownQuery = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(CooldownTags);
	const TArray<float> RemainingTimes = ASC->GetActiveEffectsTimeRemaining(CooldownQuery);
	for (const float RemainingTime : RemainingTimes)
	{
		if (RemainingTime > 0.0f)
		{
			if (OptionalRelevantTags)
			{
				OptionalRelevantTags->AddTag(ResolvedCooldownTag);
			}
			UE_LOG(LogDBACombat, Verbose, TEXT("[元素技能] 冷却中：标签=%s 剩余=%.2f 秒。"), *ResolvedCooldownTag.ToString(), RemainingTime);
			return false;
		}
	}

	return true;
}

void UDBAElementAbilityBase::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	const float RuntimeCooldownDuration = ResolveRuntimeCooldownDuration(Handle, ActorInfo);
	if (RuntimeCooldownDuration <= 0.0f)
	{
		return;
	}

	const FGameplayTag ResolvedCooldownTag = ResolveCooldownTag(Handle, ActorInfo);
	if (!ResolvedCooldownTag.IsValid())
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[元素技能] 应用冷却失败：未配置或无法解析冷却标签。"));
		return;
	}

	TSubclassOf<UGameplayEffect> ResolvedCooldownGameplayEffectClass = ResolveRuntimeCooldownGameplayEffectClass(Handle, ActorInfo);
	if (!ResolvedCooldownGameplayEffectClass)
	{
		ResolvedCooldownGameplayEffectClass = UDBAGE_Cooldown::StaticClass();
	}

	FGameplayEffectSpecHandle CooldownSpecHandle = MakeOutgoingGameplayEffectSpec(
		Handle,
		ActorInfo,
		ActivationInfo,
		ResolvedCooldownGameplayEffectClass,
		GetAbilityLevel(Handle, ActorInfo));

	if (!CooldownSpecHandle.IsValid() || !CooldownSpecHandle.Data.IsValid())
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[元素技能] 应用冷却失败：无法创建冷却 GameplayEffect。"));
		return;
	}

	CooldownSpecHandle.Data->SetDuration(RuntimeCooldownDuration, true);
	CooldownSpecHandle.Data->DynamicGrantedTags.AddTag(ResolvedCooldownTag);
	ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, CooldownSpecHandle);

	UE_LOG(LogDBACombat, Verbose, TEXT("[元素技能] 已应用冷却：标签=%s 时长=%.2f 秒。"), *ResolvedCooldownTag.ToString(), RuntimeCooldownDuration);
}

void UDBAElementAbilityBase::GetCooldownTimeRemainingAndDuration(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, float& TimeRemaining, float& InCooldownDuration) const
{
	Super::GetCooldownTimeRemainingAndDuration(Handle, ActorInfo, TimeRemaining, InCooldownDuration);
	if (TimeRemaining > 0.0f)
	{
		return;
	}

	UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	const FGameplayTag ResolvedCooldownTag = ResolveCooldownTag(Handle, ActorInfo);
	if (!ASC || !ResolvedCooldownTag.IsValid())
	{
		return;
	}

	FGameplayTagContainer CooldownTags;
	CooldownTags.AddTag(ResolvedCooldownTag);
	const FGameplayEffectQuery CooldownQuery = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(CooldownTags);
	const TArray<TPair<float, float>> DurationAndTimeRemaining = ASC->GetActiveEffectsTimeRemainingAndDuration(CooldownQuery);
	if (DurationAndTimeRemaining.Num() <= 0)
	{
		return;
	}

	int32 BestIndex = 0;
	float LongestTimeRemaining = DurationAndTimeRemaining[0].Key;
	for (int32 Index = 1; Index < DurationAndTimeRemaining.Num(); ++Index)
	{
		if (DurationAndTimeRemaining[Index].Key > LongestTimeRemaining)
		{
			LongestTimeRemaining = DurationAndTimeRemaining[Index].Key;
			BestIndex = Index;
		}
	}

	TimeRemaining = DurationAndTimeRemaining[BestIndex].Key;
	InCooldownDuration = DurationAndTimeRemaining[BestIndex].Value;
}

const FDBAAbilityRuntimeConfig* UDBAElementAbilityBase::ResolveRuntimeConfig(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const
{
	UDBAAbilitySystemComponent* DBAASC = ActorInfo ? Cast<UDBAAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get()) : nullptr;
	const FGameplayAbilitySpec* Spec = DBAASC ? DBAASC->FindAbilitySpecFromHandle(Handle) : nullptr;
	return Spec ? DBAASC->FindAbilityRuntimeConfigByInputID(Spec->InputID) : nullptr;
}

float UDBAElementAbilityBase::ResolveRuntimeEnergyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const
{
	const FDBAAbilityRuntimeConfig* RuntimeConfig = ResolveRuntimeConfig(Handle, ActorInfo);
	return RuntimeConfig ? RuntimeConfig->EnergyCost : FMath::Max(AbilityEnergyCost, EnergyCost);
}

float UDBAElementAbilityBase::ResolveRuntimeCooldownDuration(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const
{
	const FDBAAbilityRuntimeConfig* RuntimeConfig = ResolveRuntimeConfig(Handle, ActorInfo);
	return RuntimeConfig && RuntimeConfig->CooldownDuration > 0.0f ? RuntimeConfig->CooldownDuration : CooldownDuration;
}

TSubclassOf<UGameplayEffect> UDBAElementAbilityBase::ResolveRuntimeCostGameplayEffectClass(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const
{
	const FDBAAbilityRuntimeConfig* RuntimeConfig = ResolveRuntimeConfig(Handle, ActorInfo);
	if (RuntimeConfig && RuntimeConfig->CostGameplayEffectClass)
	{
		return RuntimeConfig->CostGameplayEffectClass;
	}

	return GetCostGameplayEffect() ? GetCostGameplayEffect()->GetClass() : nullptr;
}

TSubclassOf<UGameplayEffect> UDBAElementAbilityBase::ResolveRuntimeCooldownGameplayEffectClass(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const
{
	const FDBAAbilityRuntimeConfig* RuntimeConfig = ResolveRuntimeConfig(Handle, ActorInfo);
	if (RuntimeConfig && RuntimeConfig->CooldownGameplayEffectClass)
	{
		return RuntimeConfig->CooldownGameplayEffectClass;
	}

	return GetCooldownGameplayEffect() ? GetCooldownGameplayEffect()->GetClass() : nullptr;
}

FGameplayTag UDBAElementAbilityBase::ResolveCooldownTag(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const
{
	const FDBAAbilityRuntimeConfig* RuntimeConfig = ResolveRuntimeConfig(Handle, ActorInfo);
	if (RuntimeConfig && RuntimeConfig->CooldownTag.IsValid())
	{
		return RuntimeConfig->CooldownTag;
	}

	if (CooldownTag.IsValid())
	{
		return CooldownTag;
	}

	UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	const FGameplayAbilitySpec* Spec = ASC ? ASC->FindAbilitySpecFromHandle(Handle) : nullptr;
	if (!Spec)
	{
		return FGameplayTag();
	}

	const FDBAGameplayTags& GameplayTags = FDBAGameplayTags::Get();
	switch (static_cast<EDBAAbilityInputID>(Spec->InputID))
	{
	case EDBAAbilityInputID::Skill01:
		return GameplayTags.Cooldown_Skill01;
	case EDBAAbilityInputID::Skill02:
		return GameplayTags.Cooldown_Skill02;
	case EDBAAbilityInputID::Skill03:
		return GameplayTags.Cooldown_Skill03;
	case EDBAAbilityInputID::Skill04:
		return GameplayTags.Cooldown_Skill04;
	case EDBAAbilityInputID::Ultimate:
		return GameplayTags.Cooldown_Ultimate;
	default:
		return FGameplayTag();
	}
}
