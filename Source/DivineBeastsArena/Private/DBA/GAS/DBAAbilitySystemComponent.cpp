// Copyright FreeboozStudio. All Rights Reserved.
// DBA 能力系统组件实现 - 管理角色能力和战斗属性

#include "DBA/GAS/DBAAbilitySystemComponent.h"
#include "DBA/GAS/DBAAbilitySetLibrary.h"
#include "MobaBase/GAS/DBAMobaGameplayAbilityBase.h"
#include "DBA/GAS/Abilities/DBAElementAbilityBase.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemGlobals.h"
#include "Common/Types/DBACommonTypes.h"
#include "Common/DBALogChannels.h"
#include "Common/DBAInterfacesCore.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Engine/World.h"

// 构造函数 - 初始化能力系统组件
UDBAAbilitySystemComponent::UDBAAbilitySystemComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, UltimateEnergy(0.0f)      // 初始终极能量为0
	, ChainLevel(0)             // 初始连击等级为0
	, ResonanceLevel(0)          // 初始共鸣等级为0
	, LastHitTime(0.0f)         // 上次命中时间
{
	// 默认配置：启用复制
	SetIsReplicatedByDefault(true);
}

// BeginPlay - 游戏开始时初始化
void UDBAAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();

	// 服务端启动 UltimateEnergy 被动回复计时器
	if (GetOwnerRole() == ROLE_Authority)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			World->GetTimerManager().SetTimer(
				UltimateEnergyRegenTimerHandle,
				this,
				&UDBAAbilitySystemComponent::PassiveRegenUltimateEnergy,
				1.0f,    // 每秒回复1点终极能量
				true     // 循环执行
			);
		}
	}
}

// EndPlay - 游戏结束时清理
void UDBAAbilitySystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 清理所有计时器
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(UltimateEnergyRegenTimerHandle);
		World->GetTimerManager().ClearTimer(ChainResetTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

// InitAbilityActorInfo - 初始化能力Actor信息
void UDBAAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);

	// 初始化完成后可以授予 Ability
}

// GrantAbilitiesFromFixedSkillGroup - 从固定技能组授予能力
void UDBAAbilitySystemComponent::GrantAbilitiesFromFixedSkillGroup(const FName& FixedSkillGroupId)
{
	// 服务端权威
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	// 移除旧 Ability
	RemoveAllGrantedAbilities();

	// 从 FixedSkillGroupLibrary 获取 AbilitySet
	UDBAFixedSkillGroupDataAsset* AbilitySet = UDBAFixedSkillGroupLibrary::GetFixedSkillGroupById(FixedSkillGroupId);
	if (!AbilitySet)
	{
		UE_LOG(LogDBACombat, Warning, TEXT("UDBAAbilitySystemComponent::GrantAbilitiesFromFixedSkillGroup - 技能集未找到，FixedSkillGroupId: %s"), *FixedSkillGroupId.ToString());
		return;
	}

	// 授予 Passive（被动技能）
	if (AbilitySet->PassiveAbilityClass)
	{
		FGameplayAbilitySpec Spec(TSubclassOf<UGameplayAbility>(AbilitySet->PassiveAbilityClass), 1, INDEX_NONE, this);
		FGameplayAbilitySpecHandle Handle = GiveAbility(Spec);
		GrantedAbilityHandles.Add(Handle);
	}

	// 授予 Skill01~Skill04（主动技能）
	TArray<TSubclassOf<UDBAElementAbilityBase>> ActiveSkills = {
		AbilitySet->Skill01Class,
		AbilitySet->Skill02Class,
		AbilitySet->Skill03Class,
		AbilitySet->Skill04Class
	};

	// InputID 与 EDBAAbilityInputID 枚举对齐：Skill01=4, Skill02=5, Skill03=6, Skill04=7
	const int32 SkillInputIDs[] = {
		static_cast<int32>(EDBAAbilityInputID::Skill01),
		static_cast<int32>(EDBAAbilityInputID::Skill02),
		static_cast<int32>(EDBAAbilityInputID::Skill03),
		static_cast<int32>(EDBAAbilityInputID::Skill04)
	};

	// 遍历并授予每个主动技能
	for (int32 i = 0; i < ActiveSkills.Num(); ++i)
	{
		if (ActiveSkills[i])
		{
			FGameplayAbilitySpec Spec(TSubclassOf<UGameplayAbility>(ActiveSkills[i]), 1, SkillInputIDs[i], this);
			FGameplayAbilitySpecHandle Handle = GiveAbility(Spec);
			GrantedAbilityHandles.Add(Handle);
		}
	}

	// 授予 ZodiacUltimate（生肖终极技能）
	if (AbilitySet->ZodiacUltimateClass)
	{
		FGameplayAbilitySpec Spec(TSubclassOf<UGameplayAbility>(AbilitySet->ZodiacUltimateClass), 1, static_cast<int32>(EDBAAbilityInputID::Ultimate), this);
		FGameplayAbilitySpecHandle Handle = GiveAbility(Spec);
		GrantedAbilityHandles.Add(Handle);
	}

	// 授予 Resonance（共鸣能力）
	if (AbilitySet->ResonanceAbilityClass)
	{
		FGameplayAbilitySpec Spec(TSubclassOf<UGameplayAbility>(AbilitySet->ResonanceAbilityClass), 1, INDEX_NONE, this);
		FGameplayAbilitySpecHandle Handle = GiveAbility(Spec);
		GrantedAbilityHandles.Add(Handle);
	}

	// 计算 ResonanceLevel
	// 遍历所有技能类，通过 RTTI 获取元素类型，统计与主元素相同的数量
	// 注意：此实现需要在技能类构造时正确设置 ElementType 属性
	int32 SameElementCount = 0;
	EDBAElement PrimaryElement = EDBAElement::None;

	// 统计 ActiveSkills 中与主元素相同的数量
	for (const TSubclassOf<UDBAElementAbilityBase>& SkillClass : ActiveSkills)
	{
		if (SkillClass)
		{
			// 通过 CDO 获取元素类型
			if (UDBAElementAbilityBase* SkillCDO = SkillClass->GetDefaultObject<UDBAElementAbilityBase>())
			{
				EDBAElement SkillElement = SkillCDO->GetElementType();
				// 第一个非 None 的元素作为主元素
				if (PrimaryElement == EDBAElement::None && SkillElement != EDBAElement::None)
				{
					PrimaryElement = SkillElement;
				}
				// 统计与主元素相同的非 None 技能数量
				if (SkillElement == PrimaryElement && SkillElement != EDBAElement::None)
				{
					SameElementCount++;
				}
			}
		}
	}

	// 共鸣等级：2个以上同元素技能触发共鸣
	SetResonanceLevel(SameElementCount >= 5 ? 4 : SameElementCount >= 4 ? 3 : SameElementCount >= 3 ? 2 : SameElementCount >= 2 ? 1 : 0);
}

// RemoveAllGrantedAbilities - 移除所有已授予的能力
void UDBAAbilitySystemComponent::RemoveAllGrantedAbilities()
{
	// 服务端权威
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	// 清除所有已授予的能力
	for (const FGameplayAbilitySpecHandle& Handle : GrantedAbilityHandles)
	{
		ClearAbility(Handle);
	}

	GrantedAbilityHandles.Empty();
}

// AddUltimateEnergy - 增加终极能量
void UDBAAbilitySystemComponent::AddUltimateEnergy(float Amount)
{
	// 服务端权威
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	// 限制终极能量在有效范围内 [0, MaxUltimateEnergy]
	UltimateEnergy = FMath::Clamp(UltimateEnergy + Amount, 0.0f, DBAConstants::MaxUltimateEnergy);
}

// ConsumeUltimateEnergy - 消耗终极能量
bool UDBAAbilitySystemComponent::ConsumeUltimateEnergy(float Amount)
{
	// 服务端权威
	if (GetOwnerRole() != ROLE_Authority)
	{
		return false;
	}

	// 检查能量是否足够
	if (UltimateEnergy >= Amount)
	{
		UltimateEnergy -= Amount;
		return true;
	}

	return false;
}

// HasEnoughUltimateEnergy - 检查是否有足够的终极能量
bool UDBAAbilitySystemComponent::HasEnoughUltimateEnergy(float Amount) const
{
	return UltimateEnergy >= Amount;
}

// AddChainLevel - 增加连击等级
void UDBAAbilitySystemComponent::AddChainLevel(int32 Amount)
{
	// 服务端权威
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	// 限制连击等级在 [0, 10] 范围内
	ChainLevel = FMath::Clamp(ChainLevel + Amount, 0, 10);
	LastHitTime = GetWorld()->GetTimeSeconds();

	// 重置 ChainLevel 归零计时器
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(ChainResetTimerHandle);
		World->GetTimerManager().SetTimer(
			ChainResetTimerHandle,
			this,
			&UDBAAbilitySystemComponent::CheckChainReset,
			DBAConstants::ChainTimeout,
			false
		);
	}
}

// ResetChainLevel - 重置连击等级
void UDBAAbilitySystemComponent::ResetChainLevel()
{
	// 服务端权威
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	ChainLevel = 0;

	// 清理计时器
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(ChainResetTimerHandle);
	}
}

// ShouldTriggerChainFinisher - 是否应该触发连击终结
bool UDBAAbilitySystemComponent::ShouldTriggerChainFinisher() const
{
	// 连击等级达到10级时触发连击终结
	return ChainLevel >= 10;
}

// SetResonanceLevel - 设置共鸣等级
void UDBAAbilitySystemComponent::SetResonanceLevel(int32 Level)
{
	// 服务端权威
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	// 限制共鸣等级在 [0, 4] 范围内
	ResonanceLevel = FMath::Clamp(Level, 0, 4);
}

// CanActivateAbility - 检查是否可以激活指定能力
bool UDBAAbilitySystemComponent::CanActivateAbility(TSubclassOf<UDBAMobaGameplayAbilityBase> AbilityClass, AActor* Target) const
{
	// 服务端权威
	if (GetOwnerRole() != ROLE_Authority)
	{
		return false;
	}

	// 查找对应的 Ability Spec
	bool bAbilityFound = false;
	for (const FGameplayAbilitySpec& SpecIter : GetActivatableAbilities())
	{
		if (SpecIter.Ability->GetClass() == AbilityClass)
		{
			bAbilityFound = true;
			break;
		}
	}

	if (!bAbilityFound)
	{
		return false;
	}

	// 检查技能是否在冷却中
	UDBAMobaGameplayAbilityBase* AbilityCDO = AbilityClass.GetDefaultObject();
	if (AbilityCDO && AbilityCDO->IsOnCooldown())
	{
		return false;
	}

	// 检查 CurrentEnergy 是否足够（通过 AttributeSet）
	// 只有 DBAElementAbilityBase 子类才有 EnergyCost 属性
	const UDBABattleAttributeSet* AttrSet = GetSet<UDBABattleAttributeSet>();
	if (AttrSet)
	{
		if (AbilityCDO)
		{
			if (const UDBAElementAbilityBase* ElementAbility = Cast<UDBAElementAbilityBase>(AbilityCDO))
			{
				if (ElementAbility->EnergyCost > AttrSet->GetCurrentEnergy())
				{
					return false;
				}
			}
		}
	}

	// 检查目标是否合法
	if (Target && !IsValidTarget(Target, true))
	{
		return false;
	}

	return true;
}

// IsValidTarget - 检查目标是否有效（可作为技能目标）
bool UDBAAbilitySystemComponent::IsValidTarget(AActor* Target, bool bRequireEnemy) const
{
	// 服务端权威
	if (GetOwnerRole() != ROLE_Authority)
	{
		return false;
	}

	// 检查目标是否存在
	if (!Target || !IsValid(Target))
	{
		return false;
	}

	// 检查目标是否已死亡（通过 CurrentHealth）
	if (UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::Get().GetAbilitySystemComponentFromActor(Target))
	{
		if (const UDBABattleAttributeSet* TargetAttrSet = TargetASC->GetSet<UDBABattleAttributeSet>())
		{
			if (TargetAttrSet->GetCurrentHealth() <= 0.0f)
			{
				return false;
			}
		}
	}

	// 检查 TeamId 敌我关系
	if (bRequireEnemy)
	{
		// 检查目标是否实现了团队代理接口
		if (Target->Implements<UDBATeamAgentInterface>())
		{
			// 获取源角色和目标角色的团队 ID
			AActor* SourceActor = GetOwner();
			int32 SourceTeamId = -1;
			int32 TargetTeamId = -1;

			if (SourceActor && SourceActor->Implements<UDBATeamAgentInterface>())
			{
				SourceTeamId = IDBATeamAgentInterface::Execute_GetTeamId(SourceActor);
			}

			TargetTeamId = IDBATeamAgentInterface::Execute_GetTeamId(Target);

			// 中立目标（TeamId = -1）不能被作为敌人
			if (TargetTeamId == -1)
			{
				UE_LOG(LogDBACombat, Log, TEXT("[UDBAAbilitySystemComponent::IsValidTarget] 目标 %s 是中立单位，不视为敌人"), *Target->GetName());
				return false;
			}

			// 不同团队视为敌人
			if (SourceTeamId != TargetTeamId)
			{
				UE_LOG(LogDBACombat, Log, TEXT("[UDBAAbilitySystemComponent::IsValidTarget] 目标 %s 是敌人 (SourceTeam: %d, TargetTeam: %d)"), *Target->GetName(), SourceTeamId, TargetTeamId);
				return true;
			}

			// 同团队单位不是敌人
			UE_LOG(LogDBACombat, Log, TEXT("[UDBAAbilitySystemComponent::IsValidTarget] 目标 %s 是友方 (SourceTeam: %d, TargetTeam: %d)"), *Target->GetName(), SourceTeamId, TargetTeamId);
			return false;
		}

		// 如果目标没有实现团队接口，保守处理（不视为敌人）
		UE_LOG(LogDBACombat, Warning, TEXT("[UDBAAbilitySystemComponent::IsValidTarget] 目标 %s 未实现团队接口，保守返回 false"), *Target->GetName());
		return false;
	}

	return true;
}

// TriggerGameplayCue - 触发 GameplayCue 效果
void UDBAAbilitySystemComponent::TriggerGameplayCue(const FGameplayTag& CueTag, AActor* Target)
{
	if (!CueTag.IsValid())
	{
		UE_LOG(LogDBACombat, Warning, TEXT("UDBAAbilitySystemComponent::TriggerGameplayCue - CueTag 无效"));
		return;
	}

	FGameplayCueParameters CueParams;
	CueParams.Instigator = GetOwner();
	CueParams.EffectCauser = Target;

	// 触发 GameplayCue
	ExecuteGameplayCue(CueTag, CueParams);

	UE_LOG(LogDBACombat, Log, TEXT("UDBAAbilitySystemComponent::TriggerGameplayCue - Cue: %s, Target: %s"),
		*CueTag.ToString(), Target ? *Target->GetName() : TEXT("None"));
}

// PassiveRegenUltimateEnergy - 被动回复终极能量（定时器回调）
void UDBAAbilitySystemComponent::PassiveRegenUltimateEnergy()
{
	// 服务端权威
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	// 每秒回复1点终极能量
	AddUltimateEnergy(1.0f);
}

// CheckChainReset - 检查是否需要重置连击等级（定时器回调）
void UDBAAbilitySystemComponent::CheckChainReset()
{
	// 服务端权威
	if (GetOwnerRole() != ROLE_Authority)
	{
		return;
	}

	float CurrentTime = GetWorld()->GetTimeSeconds();
	// 检查距离上次命中是否超过超时时间
	if (CurrentTime - LastHitTime >= DBAConstants::ChainTimeout)
	{
		ResetChainLevel();
	}
}

// GetLifetimeReplicatedProps - 网络复制属性定义
void UDBAAbilitySystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 复制终极能量、连击等级、共鸣等级到所有客户端
	DOREPLIFETIME(UDBAAbilitySystemComponent, UltimateEnergy);
	DOREPLIFETIME(UDBAAbilitySystemComponent, ChainLevel);
	DOREPLIFETIME(UDBAAbilitySystemComponent, ResonanceLevel);
}