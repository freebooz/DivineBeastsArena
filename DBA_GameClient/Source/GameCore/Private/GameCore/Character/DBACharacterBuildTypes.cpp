// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：实现角色创建构建摘要的稳定命名与解耦解析规则。
- 阅读重点：FixedSkillGroupId 只由 Zodiac + Element 生成，FiveCamp 不参与技能组推导。
- 修改提示：保持纯基础层逻辑；不要在此加载 DataTable、PrimaryAsset、Ability 或表现资源。
*/

#include "GameCore/Character/DBACharacterBuildTypes.h"

namespace DBACharacterBuild
{
const TCHAR* ToStableZodiacName(EDBAZodiac Zodiac)
{
	switch (Zodiac)
	{
	case EDBAZodiac::Rat: return TEXT("Rat");
	case EDBAZodiac::Ox: return TEXT("Ox");
	case EDBAZodiac::Tiger: return TEXT("Tiger");
	case EDBAZodiac::Rabbit: return TEXT("Rabbit");
	case EDBAZodiac::Dragon: return TEXT("Dragon");
	case EDBAZodiac::Snake: return TEXT("Snake");
	case EDBAZodiac::Horse: return TEXT("Horse");
	case EDBAZodiac::Goat: return TEXT("Goat");
	case EDBAZodiac::Monkey: return TEXT("Monkey");
	case EDBAZodiac::Rooster: return TEXT("Rooster");
	case EDBAZodiac::Dog: return TEXT("Dog");
	case EDBAZodiac::Pig: return TEXT("Pig");
	default: return TEXT("None");
	}
}

const TCHAR* ToStableElementName(EDBAElement Element)
{
	switch (Element)
	{
	case EDBAElement::Fire: return TEXT("Fire");
	case EDBAElement::Water: return TEXT("Water");
	case EDBAElement::Wood: return TEXT("Wood");
	case EDBAElement::Gold: return TEXT("Gold");
	case EDBAElement::Earth: return TEXT("Earth");
	default: return TEXT("None");
	}
}

FName MakeFixedSkillGroupId(EDBAZodiac Zodiac, EDBAElement Element)
{
	if (Zodiac == EDBAZodiac::None || Element == EDBAElement::None)
	{
		return NAME_None;
	}

	return FName(*FString::Printf(TEXT("%s_%s"), ToStableZodiacName(Zodiac), ToStableElementName(Element)));
}

EDBAFiveCamp ResolveFiveCamp(EDBAFiveCamp RequestedFiveCamp, EDBAElement Element)
{
	if (RequestedFiveCamp != EDBAFiveCamp::None)
	{
		return RequestedFiveCamp;
	}

	switch (Element)
	{
	case EDBAElement::Fire:
		return EDBAFiveCamp::South;
	case EDBAElement::Water:
		return EDBAFiveCamp::North;
	case EDBAElement::Wood:
		return EDBAFiveCamp::East;
	case EDBAElement::Gold:
		return EDBAFiveCamp::West;
	case EDBAElement::Earth:
		return EDBAFiveCamp::Center;
	default:
		return EDBAFiveCamp::None;
	}
}

FDBACharacterBuildSummary MakeBuildSummary(
	EDBAZodiac Zodiac,
	EDBAElement PrimaryElement,
	EDBAFiveCamp RequestedFiveCamp)
{
	FDBACharacterBuildSummary Summary;
	Summary.Zodiac = Zodiac;
	Summary.PrimaryElement = PrimaryElement;
	Summary.FiveCamp = ResolveFiveCamp(RequestedFiveCamp, PrimaryElement);
	Summary.FixedSkillGroupId = MakeFixedSkillGroupId(Zodiac, PrimaryElement);
	return Summary;
}
}
