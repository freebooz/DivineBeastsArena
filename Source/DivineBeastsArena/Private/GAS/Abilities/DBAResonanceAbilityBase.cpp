// Copyright FreeboozStudio. All Rights Reserved.
// 共鸣能力基类实现 - 处理角色共鸣效果的能力基类

#include "GAS/Abilities/DBAResonanceAbilityBase.h"
#include "GAS/DBAAbilitySystemComponent.h"
#include "GAS/Effects/DBEResonanceBuffEffect.h"
#include "Common/DBALogChannels.h"
#include "Data/DBAResonanceBonusData.h"

// 构造函数 - 初始化共鸣能力
UDBAResonanceAbilityBase::UDBAResonanceAbilityBase()
{
    // Resonance 被动无需网络同步动画等
}

// ActivateAbility - 能力激活时的处理
void UDBAResonanceAbilityBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    // 调用父类实现
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 获取能力系统组件
	UDBAAbilitySystemComponent* ASC = Cast<UDBAAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());

	// 仅在网络权威（服务器）上应用共鸣效果
	if (ASC && ActorInfo->IsNetAuthority())
	{
		ApplyResonanceEffect(ASC->GetResonanceLevel());
	}
}

// ApplyResonanceEffect - 应用共鸣效果到角色
void UDBAResonanceAbilityBase::ApplyResonanceEffect(int32 CurrentResonanceLevel)
{
	// 无效的共鸣等级，不处理
	if (CurrentResonanceLevel <= 0)
	{
		UE_LOG(LogDBACombat, Log, TEXT("[DBAResonanceAbilityBase] 共鸣等级 0，无共鸣效果"));
		return;
	}

	// 获取英雄的主元素类型（用于共鸣效果视觉表现等）
	// 注意：共鸣属性加成不依赖元素类型，仅依赖共鸣等级
	// 未来可通过以下方式获取主元素：
	// 1. 从 DBAZodiacHeroDataAsset 的英雄配置中获取
	// 2. 从当前加载的技能组 FDBAZodiacElementFixedSkillGroupRow::ResonanceElement 获取
	// 3. 通过 DBAHeroSelectSubsystem 获取当前选中英雄的主元素
	EDBAElement ElementType = EDBAElement::None;

	// 通过 GameplayEffect 应用共鸣属性修饰符
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[DBAResonanceAbilityBase] 无法获取 ASC"));
		return;
	}

	// 创建共鸣效果 GE 实例
	UDBEResonanceBuffEffect* ResonanceGE = NewObject<UDBEResonanceBuffEffect>();
	if (!ResonanceGE)
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[DBAResonanceAbilityBase] 无法创建共鸣效果 GE 实例"));
		return;
	}

	// 根据共鸣等级配置 GE
	ResonanceGE->ConfigureForResonanceLevel(CurrentResonanceLevel);

	// 创建 Effect Spec
	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(ResonanceGE->GetClass(), 1, EffectContext);
	if (!SpecHandle.IsValid())
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[DBAResonanceAbilityBase] 无法创建共鸣效果 Spec"));
		return;
	}

	// 应用到自身
	FActiveGameplayEffectHandle ActiveHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

	if (ActiveHandle.IsValid())
	{
		UE_LOG(LogDBACombat, Log, TEXT("[DBAResonanceAbilityBase] 共鸣等级 %d 效果已应用，ActiveHandle: %s"),
			CurrentResonanceLevel, *ActiveHandle.ToString());
	}
	else
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[DBAResonanceAbilityBase] 共鸣等级 %d 效果应用失败"), CurrentResonanceLevel);
	}
}