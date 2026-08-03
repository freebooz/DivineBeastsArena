// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：实现 GAS 战斗属性默认值数据资产的数据完整性校验。
- 修改提示：校验只检查数据合法性，不写入运行时属性，不访问外部服务。
*/

#include "GameDBA/Gameplay/Progression/Attributes/DBABattleAttributeDefaultsDataAsset.h"

namespace
{
	void AddBattleAttributeDefaultError(TArray<FString>& OutErrors, const TCHAR* FieldName, const TCHAR* Reason)
	{
		OutErrors.Add(FString::Printf(TEXT("战斗属性默认值校验失败：字段=%s 原因=%s"), FieldName, Reason));
	}
}

bool UDBABattleAttributeDefaultsDataAsset::ValidateData_Implementation(TArray<FString>& OutErrors) const
{
	const int32 OriginalErrorCount = OutErrors.Num();

	if (Defaults.MaxHealth <= 0.0f)
	{
		AddBattleAttributeDefaultError(OutErrors, TEXT("MaxHealth"), TEXT("最大生命值必须大于 0"));
	}
	if (Defaults.CurrentHealth < 0.0f || Defaults.CurrentHealth > Defaults.MaxHealth)
	{
		AddBattleAttributeDefaultError(OutErrors, TEXT("CurrentHealth"), TEXT("当前生命值必须位于 0 到最大生命值之间"));
	}
	if (Defaults.AttackPower < 0.0f)
	{
		AddBattleAttributeDefaultError(OutErrors, TEXT("AttackPower"), TEXT("攻击力不能为负数"));
	}
	if (Defaults.Defense < 0.0f)
	{
		AddBattleAttributeDefaultError(OutErrors, TEXT("Defense"), TEXT("防御力不能为负数"));
	}
	if (Defaults.MoveSpeed <= 0.0f)
	{
		AddBattleAttributeDefaultError(OutErrors, TEXT("MoveSpeed"), TEXT("移动速度必须大于 0"));
	}
	if (Defaults.MaxEnergy <= 0.0f)
	{
		AddBattleAttributeDefaultError(OutErrors, TEXT("MaxEnergy"), TEXT("最大能量必须大于 0"));
	}
	if (Defaults.CurrentEnergy < 0.0f || Defaults.CurrentEnergy > Defaults.MaxEnergy)
	{
		AddBattleAttributeDefaultError(OutErrors, TEXT("CurrentEnergy"), TEXT("当前能量必须位于 0 到最大能量之间"));
	}
	if (Defaults.EnergyRegen < 0.0f)
	{
		AddBattleAttributeDefaultError(OutErrors, TEXT("EnergyRegen"), TEXT("能量回复不能为负数"));
	}
	if (Defaults.CriticalRate < 0.0f || Defaults.CriticalRate > 1.0f)
	{
		AddBattleAttributeDefaultError(OutErrors, TEXT("CriticalRate"), TEXT("暴击率必须位于 0 到 1 之间"));
	}
	if (Defaults.CriticalMultiplier < 1.0f)
	{
		AddBattleAttributeDefaultError(OutErrors, TEXT("CriticalMultiplier"), TEXT("暴击倍率必须大于等于 1"));
	}
	if (Defaults.MaxShield < 0.0f)
	{
		AddBattleAttributeDefaultError(OutErrors, TEXT("MaxShield"), TEXT("最大护盾不能为负数"));
	}
	if (Defaults.CurrentShield < 0.0f || Defaults.CurrentShield > Defaults.MaxShield)
	{
		AddBattleAttributeDefaultError(OutErrors, TEXT("CurrentShield"), TEXT("当前护盾必须位于 0 到最大护盾之间"));
	}

	return OutErrors.Num() == OriginalErrorCount;
}
