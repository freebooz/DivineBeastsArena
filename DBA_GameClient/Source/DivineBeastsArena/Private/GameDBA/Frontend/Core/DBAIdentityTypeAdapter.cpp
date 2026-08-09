// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/Core/DBAIdentityTypeAdapter.h"

namespace DBAIdentityTypeAdapter
{
	EDBAZodiac ToCanonical(const EDBAZodiacType ZodiacType)
	{
		const uint8 Value = static_cast<uint8>(ZodiacType);
		return Value >= static_cast<uint8>(EDBAZodiac::Rat) && Value <= static_cast<uint8>(EDBAZodiac::Pig)
			? static_cast<EDBAZodiac>(Value)
			: EDBAZodiac::None;
	}

	EDBAZodiacType ToLegacy(const EDBAZodiac Zodiac)
	{
		const uint8 Value = static_cast<uint8>(Zodiac);
		return Value >= static_cast<uint8>(EDBAZodiac::Rat) && Value <= static_cast<uint8>(EDBAZodiac::Pig)
			? static_cast<EDBAZodiacType>(Value)
			: EDBAZodiacType::None;
	}

	EDBAElement ToCanonical(const EDBAElementType ElementType)
	{
		switch (ElementType)
		{
		case EDBAElementType::Metal: return EDBAElement::Gold;
		case EDBAElementType::Wood: return EDBAElement::Wood;
		case EDBAElementType::Water: return EDBAElement::Water;
		case EDBAElementType::Fire: return EDBAElement::Fire;
		case EDBAElementType::Earth: return EDBAElement::Earth;
		default: return EDBAElement::None;
		}
	}

	EDBAElementType ToLegacy(const EDBAElement Element)
	{
		switch (Element)
		{
		case EDBAElement::Gold: return EDBAElementType::Metal;
		case EDBAElement::Wood: return EDBAElementType::Wood;
		case EDBAElement::Water: return EDBAElementType::Water;
		case EDBAElement::Fire: return EDBAElementType::Fire;
		case EDBAElement::Earth: return EDBAElementType::Earth;
		default: return EDBAElementType::None;
		}
	}

	EDBAFiveCamp ToCanonical(const EDBAFiveCampType FiveCampType)
	{
		switch (FiveCampType)
		{
		case EDBAFiveCampType::QingLong: return EDBAFiveCamp::East;
		case EDBAFiveCampType::BaiHu: return EDBAFiveCamp::West;
		case EDBAFiveCampType::ZhuQue: return EDBAFiveCamp::South;
		case EDBAFiveCampType::XuanWu: return EDBAFiveCamp::North;
		case EDBAFiveCampType::QiLin: return EDBAFiveCamp::Center;
		default: return EDBAFiveCamp::None;
		}
	}

	EDBAFiveCampType ToLegacy(const EDBAFiveCamp FiveCamp)
	{
		switch (FiveCamp)
		{
		case EDBAFiveCamp::East: return EDBAFiveCampType::QingLong;
		case EDBAFiveCamp::West: return EDBAFiveCampType::BaiHu;
		case EDBAFiveCamp::South: return EDBAFiveCampType::ZhuQue;
		case EDBAFiveCamp::North: return EDBAFiveCampType::XuanWu;
		case EDBAFiveCamp::Center: return EDBAFiveCampType::QiLin;
		default: return EDBAFiveCampType::None;
		}
	}
}
