// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Gameplay/Progression/Attributes/DBABattleAttributeSet.h"
#include "GameDBA/Core/DBAConstants.h"
#include "GameDBA/Core/DBALogChannels.h"
#include "GameDBA/Gameplay/Progression/Attributes/DBABattleAttributeDefaultsDataAsset.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

UDBABattleAttributeSet::UDBABattleAttributeSet()
{
}

void UDBABattleAttributeSet::ApplyDefaultAttributes(const UDBABattleAttributeDefaultsDataAsset* DefaultsData)
{
	if (!DefaultsData)
	{
		return;
	}

	TArray<FString> ValidationErrors;
	if (!DefaultsData->Execute_ValidateData(DefaultsData, ValidationErrors))
	{
		for (const FString& ValidationError : ValidationErrors)
		{
			UE_LOG(LogDBACombat, Error, TEXT("[战斗属性集] 默认属性数据资产校验失败：%s"), *ValidationError);
		}
		return;
	}

	const FDBABattleAttributeDefaults& Defaults = DefaultsData->GetDefaults();
	InitMaxHealth(Defaults.MaxHealth);
	InitCurrentHealth(FMath::Clamp(Defaults.CurrentHealth, 0.0f, Defaults.MaxHealth));
	InitAttackPower(Defaults.AttackPower);
	InitDefense(Defaults.Defense);
	InitMoveSpeed(Defaults.MoveSpeed);
	InitMaxEnergy(Defaults.MaxEnergy);
	InitCurrentEnergy(FMath::Clamp(Defaults.CurrentEnergy, 0.0f, Defaults.MaxEnergy));
	InitEnergyRegen(Defaults.EnergyRegen);
	InitCriticalRate(FMath::Clamp(Defaults.CriticalRate, 0.0f, 1.0f));
	InitCriticalMultiplier(FMath::Max(Defaults.CriticalMultiplier, 1.0f));
	InitMaxShield(Defaults.MaxShield);
	InitCurrentShield(FMath::Clamp(Defaults.CurrentShield, 0.0f, Defaults.MaxShield));
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
	return DefenseValue / (DefenseValue + DBAConstants::DefenseReductionConstant);
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
