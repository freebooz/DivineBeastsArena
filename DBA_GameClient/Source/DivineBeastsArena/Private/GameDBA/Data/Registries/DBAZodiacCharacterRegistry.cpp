// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：UDBAZodiacCharacterRegistry 数据资产的实现文件，提供生肖到角色类的查询功能。
- 阅读重点：先看 GetCharacterClassForZodiac 实现，理解如何从映射表查询角色类。
- 修改提示：新增查询逻辑时保持数据驱动原则，不引入硬编码 switch-case。
*/

#include "GameDBA/Data/Registries/DBAZodiacCharacterRegistry.h"
#include "GameCore/Core/DBALogChannels.h"

TSubclassOf<ADBAZodiacCharacterBase> UDBAZodiacCharacterRegistry::GetCharacterClassForZodiac(EDBAZodiac Zodiac) const
{
	if (const TSubclassOf<ADBAZodiacCharacterBase>* FoundClass = ZodiacCharacterClassMap.Find(Zodiac))
	{
		return *FoundClass;
	}

	UE_LOG(LogDBAData, Warning, TEXT("[生肖角色注册表] 未找到生肖类型 %d 对应的角色类，请在数据资产中配置映射。"), static_cast<int32>(Zodiac));
	return nullptr;
}

bool UDBAZodiacCharacterRegistry::GetPresentationDefinitionForZodiac(
	EDBAZodiac Zodiac,
	FDBAZodiacCharacterPresentationDefinition& OutDefinition) const
{
	OutDefinition = FDBAZodiacCharacterPresentationDefinition();
	if (const FDBAZodiacCharacterPresentationDefinition* FoundDefinition = ZodiacPresentationMap.Find(Zodiac))
	{
		OutDefinition = *FoundDefinition;
		return true;
	}

	UE_LOG(LogDBAData, Error, TEXT("[生肖角色注册表] 未找到生肖类型 %d 的表现资源定义。"), static_cast<int32>(Zodiac));
	return false;
}
