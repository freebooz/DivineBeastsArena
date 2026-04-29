// Copyright FreeboozStudio. All Rights Reserved.

#include "DBA/GAS/Attributes/DBABattleAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

UDBABattleAttributeSet::UDBABattleAttributeSet()
{
	// 初始化战斗属性默认值
	InitMaxHealth(1000.0f);
	InitCurrentHealth(1000.0f);
	InitAttackPower(100.0f);
	InitDefense(50.0f);
	InitMoveSpeed(600.0f);
	InitMaxEnergy(100.0f);
	InitCurrentEnergy(100.0f);
	InitEnergyRegen(5.0f);
	InitCriticalRate(0.1f);
	InitCriticalMultiplier(2.0f);
	InitMaxShield(0.0f);
	InitCurrentShield(0.0f);
}

void UDBABattleAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UDBABattleAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDBABattleAttributeSet, CurrentHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDBABattleAttributeSet, AttackPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDBABattleAttributeSet, Defense, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDBABattleAttributeSet, MoveSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDBABattleAttributeSet, MaxEnergy, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDBABattleAttributeSet, CurrentEnergy, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDBABattleAttributeSet, EnergyRegen, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDBABattleAttributeSet, CriticalRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDBABattleAttributeSet, CriticalMultiplier, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDBABattleAttributeSet, MaxShield, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDBABattleAttributeSet, CurrentShield, COND_None, REPNOTIFY_Always);
}

void UDBABattleAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// Clamp CurrentHealth：不能小于 0，不能超过 MaxHealth
	if (Attribute == GetCurrentHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	// Clamp CurrentEnergy：不能小于 0，不能超过 MaxEnergy
	else if (Attribute == GetCurrentEnergyAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxEnergy());
	}
	// Clamp CurrentShield：不能小于 0，不能超过 MaxShield
	else if (Attribute == GetCurrentShieldAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxShield());
	}
	// Clamp CriticalRate：范围 0~1
	else if (Attribute == GetCriticalRateAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, 1.0f);
	}
	// CriticalMultiplier 至少为 1.0
	else if (Attribute == GetCriticalMultiplierAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
}

void UDBABattleAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// 属性修改后再次 Clamp，确保数据一致性
	if (Data.EvaluatedData.Attribute == GetCurrentHealthAttribute())
	{
		SetCurrentHealth(FMath::Clamp(GetCurrentHealth(), 0.0f, GetMaxHealth()));
	}
	else if (Data.EvaluatedData.Attribute == GetCurrentEnergyAttribute())
	{
		SetCurrentEnergy(FMath::Clamp(GetCurrentEnergy(), 0.0f, GetMaxEnergy()));
	}
	else if (Data.EvaluatedData.Attribute == GetCurrentShieldAttribute())
	{
		SetCurrentShield(FMath::Clamp(GetCurrentShield(), 0.0f, GetMaxShield()));
	}
}

float UDBABattleAttributeSet::CalculatePhysicalDamageReduction() const
{
	float DefenseValue = GetDefense();
	return DefenseValue / (DefenseValue + 100.0f);
}

bool UDBABattleAttributeSet::RollCriticalHit() const
{
	float RandomValue = FMath::FRand();
	return RandomValue < GetCriticalRate();
}

// ========== OnRep 函数实现 ==========

void UDBABattleAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDBABattleAttributeSet, MaxHealth, OldMaxHealth);
}

void UDBABattleAttributeSet::OnRep_CurrentHealth(const FGameplayAttributeData& OldCurrentHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDBABattleAttributeSet, CurrentHealth, OldCurrentHealth);
}

void UDBABattleAttributeSet::OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDBABattleAttributeSet, AttackPower, OldAttackPower);
}

void UDBABattleAttributeSet::OnRep_Defense(const FGameplayAttributeData& OldDefense)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDBABattleAttributeSet, Defense, OldDefense);
}

void UDBABattleAttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDBABattleAttributeSet, MoveSpeed, OldMoveSpeed);
}

void UDBABattleAttributeSet::OnRep_MaxEnergy(const FGameplayAttributeData& OldMaxEnergy)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDBABattleAttributeSet, MaxEnergy, OldMaxEnergy);
}

void UDBABattleAttributeSet::OnRep_CurrentEnergy(const FGameplayAttributeData& OldCurrentEnergy)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDBABattleAttributeSet, CurrentEnergy, OldCurrentEnergy);
}

void UDBABattleAttributeSet::OnRep_EnergyRegen(const FGameplayAttributeData& OldEnergyRegen)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDBABattleAttributeSet, EnergyRegen, OldEnergyRegen);
}

void UDBABattleAttributeSet::OnRep_CriticalRate(const FGameplayAttributeData& OldCriticalRate)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDBABattleAttributeSet, CriticalRate, OldCriticalRate);
}

void UDBABattleAttributeSet::OnRep_CriticalMultiplier(const FGameplayAttributeData& OldCriticalMultiplier)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDBABattleAttributeSet, CriticalMultiplier, OldCriticalMultiplier);
}

void UDBABattleAttributeSet::OnRep_MaxShield(const FGameplayAttributeData& OldMaxShield)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDBABattleAttributeSet, MaxShield, OldMaxShield);
}

void UDBABattleAttributeSet::OnRep_CurrentShield(const FGameplayAttributeData& OldCurrentShield)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDBABattleAttributeSet, CurrentShield, OldCurrentShield);
}
