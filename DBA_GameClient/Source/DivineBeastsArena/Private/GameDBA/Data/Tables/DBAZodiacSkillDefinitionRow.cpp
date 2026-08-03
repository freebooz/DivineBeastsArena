// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：V2 生肖战术技能定义行名与枚举名称工具函数。
*/

#include "GameDBA/Data/Tables/DBAZodiacSkillDefinitionRow.h"

FName BuildZodiacSkillDefinitionRowName(const FName CharacterCodename, const EDBAZodiacTacticalSkillType SkillType)
{
	const TCHAR* TypeName = GetZodiacTacticalSkillTypeName(SkillType);
	if (TypeName == nullptr || CharacterCodename.IsNone())
	{
		return NAME_None;
	}
	return FName(*FString::Printf(TEXT("%s_%s"), *CharacterCodename.ToString(), TypeName));
}

const TCHAR* GetZodiacTacticalSkillTypeName(const EDBAZodiacTacticalSkillType SkillType)
{
	switch (SkillType)
	{
	case EDBAZodiacTacticalSkillType::Attack: return TEXT("Attack");
	case EDBAZodiacTacticalSkillType::Move: return TEXT("Move");
	case EDBAZodiacTacticalSkillType::Control: return TEXT("Control");
	case EDBAZodiacTacticalSkillType::Function: return TEXT("Function");
	case EDBAZodiacTacticalSkillType::Defense: return TEXT("Defense");
	default: return TEXT("None");
	}
}
