// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Combat/DBADamageCalculator.h"
#include "GameDBA/Core/DBAConstants.h"

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
	// 攻击方克防守方时倍率为1.2，被克制时为0.8

	const EDBAElement CounterMap[5] = {
		EDBAElement::Fire,      // 金克木
		EDBAElement::Wood,      // 木克土
		EDBAElement::Earth,     // 土克水
		EDBAElement::Water,     // 水克火
		EDBAElement::Gold        // 火克金
	};

	// 检查攻击方是否克制防守方
	for (int32 i = 0; i < 5; ++i)
	{
		if (AttackElement == CounterMap[i])
		{
			EDBAElement DefendedElement = CounterMap[(i + 1) % 5];
			if (DefenseElement == DefendedElement)
			{
				return 1.2f; // 克制
			}
		}
	}

	// 检查攻击方是否被防守方克制
	for (int32 i = 0; i < 5; ++i)
	{
		if (CounterMap[i] == DefenseElement)
		{
			if (AttackElement == CounterMap[(i + 1) % 5])
			{
				return 0.8f; // 被克制
			}
		}
	}

	return 1.0f; // 无克制关系
}

float UDBADamageCalculator::GetResonanceBonus(int32 ResonanceLevel)
{
	switch (ResonanceLevel)
	{
	case 1: return 0.05f;  // +5%
	case 2: return 0.10f;  // +10%
	case 3: return 0.15f; // +15%
	case 4: return 0.20f; // +20%
	default: return 0.0f;
	}
}

float UDBADamageCalculator::GetChainMultiplier(int32 ChainLevel)
{
	if (ChainLevel >= 10)
	{
		return 1.0f; // 终结连锁不适用普通倍率
	}
	if (ChainLevel >= 6)
	{
		return 1.35f; // 135%
	}
	if (ChainLevel >= 1)
	{
		return 1.20f; // 120%
	}
	return 1.0f;
}

bool UDBADamageCalculator::IsChainFinal(int32 ChainLevel)
{
	return ChainLevel >= 10;
}