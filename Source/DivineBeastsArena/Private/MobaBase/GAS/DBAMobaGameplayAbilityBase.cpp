// Copyright FreeboozStudio. All Rights Reserved.

#include "MobaBase/GAS/DBAMobaGameplayAbilityBase.h"
#include "MobaBase/GAS/DBAMobaAbilitySystemComponentBase.h"
#include "GAS/Attributes/DBABattleAttributeSet.h"
#include "Common/DBALogChannels.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayCueManager.h"
#include "GameplayEffectTypes.h"

UDBAMobaGameplayAbilityBase::UDBAMobaGameplayAbilityBase()
{
	// 默认配置
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	bServerAuthority = true;
	bClientPrediction = false;
}

bool UDBAMobaGameplayAbilityBase::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	// 调用父类检查
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// 检查阻止 Tag
	if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		if (ActorInfo->AbilitySystemComponent->HasAnyMatchingGameplayTags(BlockTags))
		{
			return false;
		}
	}

	// 检查冷却
	if (IsOnCooldown())
	{
		return false;
	}

	return true;
}

void UDBAMobaGameplayAbilityBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 根据 ActivationPolicy 决定激活逻辑
	switch (ActivationPolicy)
	{
	case EDBAMobaAbilityActivationPolicy::OnInputTriggered:
		// 输入触发：等待输入系统调用（由 GAS 自动处理）
		// 此处仅记录日志，实际输入绑定由 InputComponent 处理
		UE_LOG(LogDBACombat, Log, TEXT("[DBAMobaGameplayAbilityBase] 输入触发激活 - AbilityTag: %s"), *AbilityTag.ToString());
		break;

	case EDBAMobaAbilityActivationPolicy::OnSpawn:
		// 生成时激活：Avatar 生成时自动激活
		UE_LOG(LogDBACombat, Log, TEXT("[DBAMobaGameplayAbilityBase] 生成时自动激活 - AbilityTag: %s"), *AbilityTag.ToString());
		break;

	case EDBAMobaAbilityActivationPolicy::Passive:
		// 被动技能：通常不通过此路径激活，被动效果由 GE 或其他机制管理
		UE_LOG(LogDBACombat, Log, TEXT("[DBAMobaGameplayAbilityBase] 被动技能激活 - AbilityTag: %s"), *AbilityTag.ToString());
		break;

	default:
		UE_LOG(LogDBACombat, Warning, TEXT("[DBAMobaGameplayAbilityBase] 未知的 ActivationPolicy: %d"), static_cast<int32>(ActivationPolicy));
		break;
	}

	// 客户端本地反馈
	// 本地控制的 Actor 需要播放本地表现（动画/音效/粒子）
	// Dedicated Server 不调用此函数（没有本地表现）
	// UE 5.7: 使用 WorldType 判断是否为专用服务器
	if (ActorInfo->IsLocallyControlled() && GetWorld()->GetNetMode() != NM_DedicatedServer)
	{
		OnClientLocalFeedback();
	}

	// 服务端权威激活
	if (HasAuthority(&ActivationInfo))
	{
		OnServerActivate();
	}
}

void UDBAMobaGameplayAbilityBase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UDBAMobaGameplayAbilityBase::OnClientLocalFeedback_Implementation()
{
	// 默认空实现，子类重写
	// 播放动画预览、音效、粒子等表现
	UE_LOG(LogDBACombat, Log, TEXT("[DBAMobaGameplayAbilityBase] OnClientLocalFeedback - AbilityTag: %s"), *AbilityTag.ToString());
}

void UDBAMobaGameplayAbilityBase::OnServerAuthorityConfirmed_Implementation()
{
	// 默认空实现，子类重写
	// 客户端接收服务端裁定结果后的处理
	UE_LOG(LogDBACombat, Log, TEXT("[DBAMobaGameplayAbilityBase] OnServerAuthorityConfirmed - AbilityTag: %s"), *AbilityTag.ToString());
}

void UDBAMobaGameplayAbilityBase::OnServerAuthorityRejected_Implementation()
{
	// 默认空实现，子类重写
	// 客户端预测失败后的回滚处理
	UE_LOG(LogDBACombat, Warning, TEXT("[DBAMobaGameplayAbilityBase] OnServerAuthorityRejected - AbilityTag: %s"), *AbilityTag.ToString());
}

void UDBAMobaGameplayAbilityBase::ExecuteGameplayCueOnTarget(FGameplayTag CueTag, const FGameplayCueParameters& Parameters)
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->ExecuteGameplayCue(CueTag, Parameters);
	}
}

UDBAMobaAbilitySystemComponentBase* UDBAMobaGameplayAbilityBase::GetDBAAbilitySystemComponent() const
{
	return Cast<UDBAMobaAbilitySystemComponentBase>(GetAbilitySystemComponentFromActorInfo());
}

bool UDBAMobaGameplayAbilityBase::IsOnCooldown() const
{
	if (CooldownTag.IsValid())
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			return ASC->HasMatchingGameplayTag(CooldownTag);
		}
	}
	return false;
}

float UDBAMobaGameplayAbilityBase::GetCooldownTimeRemaining(const FGameplayAbilityActorInfo* ActorInfo) const
{
	if (!CooldownTag.IsValid())
	{
		return 0.0f;
	}

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		// 检查冷却 Tag 是否存在
		if (ASC->HasMatchingGameplayTag(CooldownTag))
		{
			// UE 5.7 API 变更：简化处理，返回 1.0 表示有冷却
			return 1.0f;
		}
	}

	return 0.0f;
}

void UDBAMobaGameplayAbilityBase::OnServerActivate_Implementation()
{
	// 服务端权威技能激活逻辑
	// 子类应重写此函数以实现具体技能效果

	UDBAMobaAbilitySystemComponentBase* DBAAbilitySystem = GetDBAAbilitySystemComponent();
	if (!DBAAbilitySystem)
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[DBAMobaGameplayAbilityBase] OnServerActivate 失败：无法获取 DBA AbilitySystemComponent"));
		return;
	}

	// 获取当前 Actor 信息
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	if (!ActorInfo || !ActorInfo->OwnerActor.IsValid())
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[DBAMobaGameplayAbilityBase] OnServerActivate 失败：无效的 ActorInfo"));
		return;
	}

	// 日志输出技能激活信息
	UE_LOG(LogDBACombat, Log, TEXT("[DBAMobaGameplayAbilityBase] OnServerActivate - AbilityTag: %s, Owner: %s"),
		*AbilityTag.ToString(),
		*ActorInfo->OwnerActor->GetName());

	// 注意：具体技能逻辑应在子类中实现
	// 例如：
	// - 伤害计算
	// - 效果应用
	// - GameplayCue 触发
	// - 状态变化
}

float UDBAMobaGameplayAbilityBase::CalculateDamageWithDefense(float BaseDamage, float TargetDefense) const
{
	// 伤害减免公式：DamageReduction = Defense / (Defense + 100)
	// 最终伤害 = BaseDamage * (1 - DamageReduction)
	float DamageReduction = TargetDefense / (TargetDefense + 100.0f);
	float FinalDamage = BaseDamage * (1.0f - DamageReduction);

	UE_LOG(LogDBACombat, Log, TEXT("[DBAMobaGameplayAbilityBase] 计算伤害 - 基础伤害: %.1f, 防御力: %.1f, 减免: %.2f, 最终: %.1f"),
		BaseDamage, TargetDefense, DamageReduction, FinalDamage);

	return FMath::Max(FinalDamage, 0.0f);
}

float UDBAMobaGameplayAbilityBase::ApplyCriticalHit(float Damage, float CriticalRate, float CriticalMultiplier) const
{
	// 暴击判定
	// CriticalRate 范围 0~1（0% ~ 100%），已在 AttributeSet 初始化时确保范围正确
	float CritChance = FMath::Clamp(CriticalRate, 0.0f, 1.0f);

	if (FMath::FRand() < CritChance)
	{
		float FinalDamage = Damage * CriticalMultiplier;
		UE_LOG(LogDBACombat, Log, TEXT("[DBAMobaGameplayAbilityBase] 暴击！原始伤害: %.1f, 倍率: %.2f, 最终伤害: %.1f"),
			Damage, CriticalMultiplier, FinalDamage);
		return FinalDamage;
	}

	return Damage;
}

void UDBAMobaGameplayAbilityBase::ApplyDamageToTarget(AActor* TargetActor, float BaseDamage, const FGameplayTagContainer& InDamageTags)
{
	// 防御性检查：确保目标有效
	ensure(TargetActor != nullptr);

	if (!TargetActor)
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[DBAMobaGameplayAbilityBase] ApplyDamageToTarget 失败：目标为空"));
		return;
	}

	// 注意：ApplyDamageToTarget 应仅在服务端调用
	// 此函数不进行 Authority 检查，调用者需确保在服务端上下文中调用

	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::Get().GetAbilitySystemComponentFromActor(TargetActor);
	if (!TargetASC)
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[DBAMobaGameplayAbilityBase] ApplyDamageToTarget 失败：目标没有 ASC"));
		return;
	}

	// 获取目标防御力
	float TargetDefense = 0.0f;
	if (UDBAMobaAbilitySystemComponentBase* TargetDBASC = Cast<UDBAMobaAbilitySystemComponentBase>(TargetASC))
	{
		if (const UDBABattleAttributeSet* TargetAttrSet = TargetDBASC->GetSet<UDBABattleAttributeSet>())
		{
			TargetDefense = TargetAttrSet->GetDefense();
		}
	}

	// 计算最终伤害（考虑防御）
	float FinalDamage = CalculateDamageWithDefense(BaseDamage, TargetDefense);

	// 创建伤害 GameplayEffect Context
	FGameplayEffectContextHandle EffectContext = MakeEffectContext(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo());
	EffectContext.AddSourceObject(GetAvatarActorFromActorInfo());

	// 防御性检查
	ensure(EffectContext.IsValid());
	ensure(TargetActor != nullptr);

	if (TargetActor)
	{
		EffectContext.AddActors(TArray<TWeakObjectPtr<AActor>>({ TargetActor }));
	}

	// 触发伤害 GameplayCue
	FGameplayCueParameters CueParams;
	CueParams.SourceObject = GetAvatarActorFromActorInfo();
	CueParams.Instigator = GetAvatarActorFromActorInfo();
	CueParams.EffectContext = EffectContext;
	CueParams.AggregatedSourceTags = InDamageTags;
	ExecuteGameplayCueOnTarget(FGameplayTag::RequestGameplayTag(TEXT("Cue.Damage")), CueParams);

	UE_LOG(LogDBACombat, Log, TEXT("[DBAMobaGameplayAbilityBase] 对目标造成伤害 - 目标: %s, 基础伤害: %.1f, 防御力: %.1f, 最终伤害: %.1f"),
		*TargetActor->GetName(), BaseDamage, TargetDefense, FinalDamage);
}

void UDBAMobaGameplayAbilityBase::TriggerGameplayCueOnTarget(FGameplayTag CueTag, AActor* TargetActor, float Magnitude)
{
	// 防御性检查
	ensure(CueTag.IsValid());

	if (!CueTag.IsValid())
	{
		UE_LOG(LogDBACombat, Warning, TEXT("[DBAMobaGameplayAbilityBase] TriggerGameplayCueOnTarget 失败：CueTag 无效"));
		return;
	}

	FGameplayCueParameters CueParams;
	CueParams.SourceObject = GetAvatarActorFromActorInfo();
	CueParams.Instigator = GetAvatarActorFromActorInfo();

	// 设置伤害值
	CueParams.GameplayEffectLevel = 1;
	CueParams.AggregatedSourceTags = DamageTags;

	ExecuteGameplayCueOnTarget(CueTag, CueParams);

	UE_LOG(LogDBACombat, Log, TEXT("[DBAMobaGameplayAbilityBase] TriggerGameplayCue - Cue: %s, Target: %s"),
		*CueTag.ToString(), TargetActor ? *TargetActor->GetName() : TEXT("None"));
}

const AActor* UDBAMobaGameplayAbilityBase::GetTargetActorFromEventData(const FGameplayEventData& EventData) const
{
	// EventData.Target 是 TObjectPtr<AActor>
	// 返回 const 指针以避免 const_cast
	return EventData.Target.Get();
}
