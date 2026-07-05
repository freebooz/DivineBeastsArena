// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Combat/DBADamageCalculator.h"
#include "GameDBA/Core/DBAConstants.h"
#include "GameDBA/GAS/Attributes/DBABattleAttributeSet.h"
#include "GameDBA/GAS/DBAAbilitySystemComponent.h"
#include "GameDBA/Character/DBAZodiacCharacterBase.h"
#include "GameDBA/Player/DBAPlayerState.h"
#include "AbilitySystemComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"

// EDBAElement to EDBAElementType conversion map
static EDBAElementType GetElementTypeFromGoldFireWoodEarthWater(EDBAElement OldElement)
{
	switch (OldElement)
	{
	case EDBAElement::Gold: return EDBAElementType::Metal;
	case EDBAElement::Fire: return EDBAElementType::Fire;
	case EDBAElement::Wood: return EDBAElementType::Wood;
	case EDBAElement::Water: return EDBAElementType::Water;
	case EDBAElement::Earth: return EDBAElementType::Earth;
	default: return EDBAElementType::None;
	}
}

namespace
{
	ADBAPlayerState* ResolveDBAPlayerState(AActor* Actor, int32 Depth = 0)
	{
		if (!Actor || Depth > 2)
		{
			return nullptr;
		}

		if (ADBAPlayerState* DirectPlayerState = Cast<ADBAPlayerState>(Actor))
		{
			return DirectPlayerState;
		}

		if (AController* InstigatorController = Actor->GetInstigatorController())
		{
			if (ADBAPlayerState* InstigatorPlayerState = Cast<ADBAPlayerState>(InstigatorController->PlayerState))
			{
				return InstigatorPlayerState;
			}
		}

		if (APawn* Pawn = Cast<APawn>(Actor))
		{
			if (ADBAPlayerState* PawnPlayerState = Pawn->GetPlayerState<ADBAPlayerState>())
			{
				return PawnPlayerState;
			}
		}

		if (AController* Controller = Cast<AController>(Actor))
		{
			if (ADBAPlayerState* ControllerPlayerState = Cast<ADBAPlayerState>(Controller->PlayerState))
			{
				return ControllerPlayerState;
			}
		}

		return ResolveDBAPlayerState(Actor->GetOwner(), Depth + 1);
	}

	void RecordMatchEliminationStats(AActor* Attacker, ADBAZodiacCharacterBase* Victim)
	{
		if (!Victim)
		{
			return;
		}

		ADBAPlayerState* VictimPlayerState = ResolveDBAPlayerState(Victim);
		ADBAPlayerState* AttackerPlayerState = ResolveDBAPlayerState(Attacker);

		if (VictimPlayerState)
		{
			VictimPlayerState->RecordDeath();
		}

		if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
		{
			AttackerPlayerState->RecordKill();
		}
	}

	UAbilitySystemComponent* ResolveAbilitySystemComponent(AActor* Actor)
	{
		if (!Actor)
		{
			return nullptr;
		}

		if (UAbilitySystemComponent* ASC = Actor->FindComponentByClass<UAbilitySystemComponent>())
		{
			return ASC;
		}

		AActor* Owner = Actor->GetOwner();
		return Owner ? Owner->FindComponentByClass<UAbilitySystemComponent>() : nullptr;
	}

	void ExecuteDamageGameplayCue(
		AActor* Attacker,
		AActor* Target,
		float FinalDamage,
		bool bIsCritical,
		const FGameplayTag& GameplayCueTag,
		const FVector& HitLocation)
	{
		if (!GameplayCueTag.IsValid())
		{
			return;
		}

		UAbilitySystemComponent* CueASC = ResolveAbilitySystemComponent(Target);
		if (!CueASC)
		{
			CueASC = ResolveAbilitySystemComponent(Attacker);
		}
		if (!CueASC)
		{
			return;
		}

		FGameplayCueParameters CueParams;
		CueParams.Instigator = Attacker;
		CueParams.EffectCauser = Attacker;
		CueParams.SourceObject = Attacker;
		CueParams.Location = HitLocation.IsNearlyZero() && Target ? Target->GetActorLocation() : HitLocation;
		CueParams.RawMagnitude = FinalDamage;
		CueParams.NormalizedMagnitude = bIsCritical ? 1.0f : 0.0f;
		CueASC->ExecuteGameplayCue(GameplayCueTag, CueParams);
		if (UDBAAbilitySystemComponent* DBAASC = Cast<UDBAAbilitySystemComponent>(CueASC))
		{
			DBAASC->OnSkillCueExecuted.Broadcast(GameplayCueTag.GetTagName(), Target);
		}
	}
}

float UDBADamageCalculator::CalculateDamage(
	float BaseDamage,
	EDBAElement AttackElement,
	EDBAElement DefenseElement,
	int32 ResonanceLevel,
	int32 ChainLevel)
{
	float Damage = BaseDamage;

	// 1. 元素克制
	float ElementMultiplier = GetElementMultiplier(AttackElement, DefenseElement);
	Damage *= ElementMultiplier;

	// 2. 共鸣加成
	float ResonanceBonus = GetResonanceBonus(ResonanceLevel);
	Damage *= (1.0f + ResonanceBonus);

	// 3. 连锁加成 (终结技能按最大生命计算)
	if (IsChainFinal(ChainLevel))
	{
		// 终结连锁返回特殊值，实际伤害在调用处按最大生命计算
		return Damage;
	}

	float ChainMultiplier = GetChainMultiplier(ChainLevel);
	Damage *= ChainMultiplier;

	return Damage;
}

float UDBADamageCalculator::GetElementMultiplier(EDBAElement AttackElement, EDBAElement DefenseElement)
{
	if (AttackElement == EDBAElement::None || DefenseElement == EDBAElement::None)
	{
		return 1.0f;
	}

		// 五行相克: 火→金→木→土→水→火
	// 攻击方克防守方时倍率为1.25，被克制时为0.8

	const EDBAElement CounterMap[DBAConstants::ElementCount] = {
		EDBAElement::Fire,      // 金克木
		EDBAElement::Wood,      // 木克土
		EDBAElement::Earth,     // 土克水
		EDBAElement::Water,     // 水克火
		EDBAElement::Gold        // 火克金
	};

	// 检查攻击方是否克制防守方
	for (int32 i = 0; i < DBAConstants::ElementCount; ++i)
	{
		if (AttackElement == CounterMap[i])
		{
			EDBAElement DefendedElement = CounterMap[(i + 1) % DBAConstants::ElementCount];
			if (DefenseElement == DefendedElement)
			{
				return DBAConstants::ElementCounter_Normal; // 克制 1.25
			}
		}
	}

	// 检查攻击方是否被防守方克制
	for (int32 i = 0; i < DBAConstants::ElementCount; ++i)
	{
		if (CounterMap[i] == DefenseElement)
		{
			if (AttackElement == CounterMap[(i + 1) % DBAConstants::ElementCount])
			{
				return DBAConstants::ElementCountered_Normal; // 被克制 0.80
			}
		}
	}

	return DBAConstants::ElementNeutral; // 无克制关系 1.0
}

float UDBADamageCalculator::GetResonanceBonus(int32 ResonanceLevel)
{
	switch (ResonanceLevel)
	{
	case 1: return DBAConstants::ResonanceLevel1_DamageBonus;
	case 2: return DBAConstants::ResonanceLevel2_DamageBonus;
	case 3: return DBAConstants::ResonanceLevel3_DamageBonus;
	case 4: return DBAConstants::ResonanceLevel4_DamageBonus;
	default: return 0.0f;
	}
}

float UDBADamageCalculator::GetChainMultiplier(int32 ChainLevel)
{
	if (ChainLevel >= DBAConstants::MaxChainLevel)
	{
		return 1.0f; // 终结连锁不适用普通倍率
	}
	if (ChainLevel >= DBAConstants::ChainTier2Threshold)
	{
		return DBAConstants::ChainTier2DamageBonus;
	}
	if (ChainLevel >= DBAConstants::ChainTier1Threshold)
	{
		return DBAConstants::ChainTier1DamageBonus;
	}
	return 1.0f;
}

bool UDBADamageCalculator::IsChainFinal(int32 ChainLevel)
{
	return ChainLevel >= DBAConstants::MaxChainLevel;
}

float UDBADamageCalculator::CalculateFinalDamage(
	float BaseDamage,
	EDBAElement AttackElement,
	EDBAElement DefenseElement,
	int32 ResonanceLevel,
	int32 ChainLevel,
	float Defense,
	float CriticalRate,
	float CriticalMultiplier,
	bool& OutbIsCritical)
{
	// 1. 计算元素克制
	float ElementMultiplier = GetElementMultiplier(AttackElement, DefenseElement);
	float Damage = BaseDamage * ElementMultiplier;

	// 2. 计算共鸣加成
	float ResonanceBonus = GetResonanceBonus(ResonanceLevel);
	Damage *= (1.0f + ResonanceBonus);

	// 3. 计算连锁加成
	if (IsChainFinal(ChainLevel))
	{
		// 终结连锁返回特殊值，实际伤害在调用处按最大生命计算
		// 这里不做终结连锁的最终计算，由调用者处理
	}
	else
	{
		float ChainMultiplier = GetChainMultiplier(ChainLevel);
		Damage *= ChainMultiplier;
	}

	// 4. 计算防御减免
	float PhysicalReduction = Defense / (Defense + DBAConstants::DefenseReductionConstant);
	Damage *= (1.0f - PhysicalReduction);

	// 5. 暴击判定
	OutbIsCritical = FMath::FRand() < CriticalRate;
	if (OutbIsCritical)
	{
		Damage *= CriticalMultiplier;
	}

	return FMath::Max(Damage, 0.0f);
}

void UDBADamageCalculator::ApplyDamageToTarget(
	AActor* Attacker,
	AActor* Target,
	float FinalDamage,
	EDBAElement Element,
	bool bIsCritical)
{
	if (!Target || !Attacker)
	{
		return;
	}

	if (!Attacker->HasAuthority())
	{
		return;
	}

	// 获取目标的AbilitySystemComponent
	UAbilitySystemComponent* TargetASC = Target->FindComponentByClass<UAbilitySystemComponent>();
	if (!TargetASC)
	{
		return;
	}

	// 创建GameplayEffectContext用于伤害传递
	FGameplayEffectContextHandle EffectContext = TargetASC->MakeEffectContext();
	EffectContext.AddSourceObject(Attacker);

	// 应用伤害GameplayEffect
	// 注意：这里需要通过GameplayEffect来应用伤害，而不是直接修改属性
	// 具体实现取决于游戏设计的伤害效果蓝图
	FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(FName("Damage.Physical"), false);
	(void)DamageTag;

	// 临时实现：直接通过AttributeSet应用伤害
	// 在实际项目中应该通过GameplayEffect来做
	UDBABattleAttributeSet* BattleAttrSet = const_cast<UDBABattleAttributeSet*>(Cast<const UDBABattleAttributeSet>(TargetASC->GetAttributeSet(UDBABattleAttributeSet::StaticClass())));
	if (BattleAttrSet)
	{
		float CurrentHealth = BattleAttrSet->GetCurrentHealth();
		BattleAttrSet->SetCurrentHealth(FMath::Max(CurrentHealth - FinalDamage, 0.0f));
	}

	// 检查目标是否死亡
	ADBAZodiacCharacterBase* ZodiacChar = Cast<ADBAZodiacCharacterBase>(Target);
	if (ZodiacChar)
	{
		float CurrentHealth = ZodiacChar->GetCurrentHealth();
		if (CurrentHealth <= 0.0f && !ZodiacChar->IsDead())
		{
			RecordMatchEliminationStats(Attacker, ZodiacChar);
			ZodiacChar->OnDeath();
		}
	}
}

// ========== Value Object 方法实现 (EDBAElementType) ==========

void UDBADamageCalculator::ApplyDamageToTargetWithCue(
	AActor* Attacker,
	AActor* Target,
	float FinalDamage,
	EDBAElement Element,
	bool bIsCritical,
	const FGameplayTag& GameplayCueTag,
	FVector HitLocation)
{
	if (!Target || !Attacker || FinalDamage <= 0.0f)
	{
		return;
	}

	if (!Attacker->HasAuthority())
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = ResolveAbilitySystemComponent(Target);
	if (TargetASC)
	{
		FGameplayEffectContextHandle EffectContext = TargetASC->MakeEffectContext();
		EffectContext.AddSourceObject(Attacker);

		FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(FName("Damage.Physical"), false);
		(void)DamageTag;

		UDBABattleAttributeSet* BattleAttrSet = const_cast<UDBABattleAttributeSet*>(Cast<const UDBABattleAttributeSet>(TargetASC->GetAttributeSet(UDBABattleAttributeSet::StaticClass())));
		if (BattleAttrSet)
		{
			const float CurrentHealth = BattleAttrSet->GetCurrentHealth();
			BattleAttrSet->SetCurrentHealth(FMath::Max(CurrentHealth - FinalDamage, 0.0f));
		}
	}
	else
	{
		FPointDamageEvent DamageEvent;
		DamageEvent.Damage = FinalDamage;
		DamageEvent.HitInfo = FHitResult(Target, nullptr, HitLocation, FVector::ZeroVector);
		Target->TakeDamage(FinalDamage, DamageEvent, Attacker->GetInstigatorController(), Attacker);
	}

	ExecuteDamageGameplayCue(Attacker, Target, FinalDamage, bIsCritical, GameplayCueTag, HitLocation);

	ADBAZodiacCharacterBase* ZodiacChar = Cast<ADBAZodiacCharacterBase>(Target);
	if (ZodiacChar)
	{
		const float CurrentHealth = ZodiacChar->GetCurrentHealth();
		if (CurrentHealth <= 0.0f && !ZodiacChar->IsDead())
		{
			RecordMatchEliminationStats(Attacker, ZodiacChar);
			ZodiacChar->OnDeath();
		}
	}
}

EDBAElementType UDBADamageCalculator::GetElementTypeFromOldEnum(EDBAElement OldElement)
{
	return GetElementTypeFromGoldFireWoodEarthWater(OldElement);
}

FElementCounterResult UDBADamageCalculator::GetElementCounterResult(EDBAElementType AttackElement, EDBAElementType DefenseElement)
{
	FElementCounterResult Result;

	if (AttackElement == EDBAElementType::None || DefenseElement == EDBAElementType::None)
	{
		Result.Multiplier = 1.0f;
		Result.ResultType = EDBAElementCounterResult::None;
		return Result;
	}

	// 五行相克: 火→金→木→土→水→火 (Metal替代Gold)
	// 攻击方克防守方时倍率为1.25，被克制时为0.8
	const EDBAElementType CounterMap[DBAConstants::ElementCount] = {
		EDBAElementType::Fire,   // Metal克Wood
		EDBAElementType::Wood,   // Wood克Earth
		EDBAElementType::Earth,  // Earth克Water
		EDBAElementType::Water,  // Water克Fire
		EDBAElementType::Metal   // Fire克Metal
	};

	// 检查攻击方是否克制防守方
	for (int32 i = 0; i < DBAConstants::ElementCount; ++i)
	{
		if (AttackElement == CounterMap[i])
		{
			EDBAElementType DefendedElement = CounterMap[(i + 1) % DBAConstants::ElementCount];
			if (DefenseElement == DefendedElement)
			{
				Result.Multiplier = DBAConstants::ElementCounter_Normal; // 克制 1.25
				Result.ResultType = EDBAElementCounterResult::Counter;
				return Result;
			}
		}
	}

	// 检查攻击方是否被防守方克制
	for (int32 i = 0; i < DBAConstants::ElementCount; ++i)
	{
		if (CounterMap[i] == DefenseElement)
		{
			if (AttackElement == CounterMap[(i + 1) % DBAConstants::ElementCount])
			{
				Result.Multiplier = DBAConstants::ElementCountered_Normal; // 被克制 0.80
				Result.ResultType = EDBAElementCounterResult::Countered;
				return Result;
			}
		}
	}

	Result.Multiplier = DBAConstants::ElementNeutral; // 无克制关系 1.0
	Result.ResultType = EDBAElementCounterResult::None;
	return Result;
}

float UDBADamageCalculator::GetResonanceBonusForElement(EDBAResonanceLevel ResonanceLevel)
{
	switch (ResonanceLevel)
	{
	case EDBAResonanceLevel::Level1: return DBAConstants::ResonanceLevel1_DamageBonus;
	case EDBAResonanceLevel::Level2: return DBAConstants::ResonanceLevel2_DamageBonus;
	case EDBAResonanceLevel::Level3: return DBAConstants::ResonanceLevel3_DamageBonus;
	case EDBAResonanceLevel::Level4: return DBAConstants::ResonanceLevel4_DamageBonus;
	default: return 0.0f;
	}
}

FChainBonus UDBADamageCalculator::GetChainBonus(int32 ChainLevel)
{
	FChainBonus Bonus;

	if (ChainLevel >= DBAConstants::MaxChainLevel)
	{
		Bonus.Multiplier = 1.0f;
		Bonus.bIsFinal = true;
	}
	else if (ChainLevel >= DBAConstants::ChainTier2Threshold)
	{
		Bonus.Multiplier = DBAConstants::ChainTier2DamageBonus;
		Bonus.bIsFinal = false;
	}
	else if (ChainLevel >= DBAConstants::ChainTier1Threshold)
	{
		Bonus.Multiplier = DBAConstants::ChainTier1DamageBonus;
		Bonus.bIsFinal = false;
	}
	else
	{
		Bonus.Multiplier = 1.0f;
		Bonus.bIsFinal = false;
	}

	return Bonus;
}

FFinalDamageResult UDBADamageCalculator::CalculateFinalDamageWithObject(const FDamageCalculationParams& Params)
{
	FFinalDamageResult Result;

	// 1. 元素克制
	FElementCounterResult ElementResult = GetElementCounterResult(Params.AttackElement, Params.DefenseElement);
	Result.ElementResult = ElementResult;
	float Damage = Params.BaseDamage * ElementResult.Multiplier;

	// 2. 共鸣加成
	float ResonanceBonus = GetResonanceBonusForElement(static_cast<EDBAResonanceLevel>(Params.ResonanceLevel));
	Result.ResonanceBonusPercent = ResonanceBonus;
	Damage *= (1.0f + ResonanceBonus);

	// 3. 连锁加成
	FChainBonus ChainBonus = GetChainBonus(Params.ChainLevel);
	Result.ChainBonus = ChainBonus;
	if (!ChainBonus.bIsFinal)
	{
		Damage *= ChainBonus.Multiplier;
	}

	// 4. 防御减免
	float PhysicalReduction = Params.Defense / (Params.Defense + DBAConstants::DefenseReductionConstant);
	Result.DefenseReductionPercent = PhysicalReduction;
	Damage *= (1.0f - PhysicalReduction);

	// 5. 暴击判定
	Result.bIsCritical = FMath::FRand() < Params.CriticalRate;
	if (Result.bIsCritical)
	{
		Damage *= Params.CriticalMultiplier;
	}

	Result.FinalDamage = FMath::Max(Damage, 0.0f);
	return Result;
}
