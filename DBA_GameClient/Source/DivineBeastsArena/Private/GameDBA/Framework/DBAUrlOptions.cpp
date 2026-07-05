// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Framework/DBAUrlOptions.h"

#include "GenericPlatform/GenericPlatformHttp.h"

namespace DBAUrlOptions
{
	namespace
	{
		bool StableNameEquals(const FString& Value, const TCHAR* Expected)
		{
			return Value.Equals(Expected, ESearchCase::IgnoreCase);
		}

		EDBAZodiac ParseStableZodiacName(const FString& Value)
		{
			if (StableNameEquals(Value, TEXT("Rat"))) { return EDBAZodiac::Rat; }
			if (StableNameEquals(Value, TEXT("Ox"))) { return EDBAZodiac::Ox; }
			if (StableNameEquals(Value, TEXT("Tiger"))) { return EDBAZodiac::Tiger; }
			if (StableNameEquals(Value, TEXT("Rabbit"))) { return EDBAZodiac::Rabbit; }
			if (StableNameEquals(Value, TEXT("Dragon"))) { return EDBAZodiac::Dragon; }
			if (StableNameEquals(Value, TEXT("Snake"))) { return EDBAZodiac::Snake; }
			if (StableNameEquals(Value, TEXT("Horse"))) { return EDBAZodiac::Horse; }
			if (StableNameEquals(Value, TEXT("Goat"))) { return EDBAZodiac::Goat; }
			if (StableNameEquals(Value, TEXT("Monkey"))) { return EDBAZodiac::Monkey; }
			if (StableNameEquals(Value, TEXT("Rooster"))) { return EDBAZodiac::Rooster; }
			if (StableNameEquals(Value, TEXT("Dog"))) { return EDBAZodiac::Dog; }
			if (StableNameEquals(Value, TEXT("Pig"))) { return EDBAZodiac::Pig; }
			return EDBAZodiac::None;
		}

		EDBAElement ParseStableElementName(const FString& Value)
		{
			if (StableNameEquals(Value, TEXT("Fire"))) { return EDBAElement::Fire; }
			if (StableNameEquals(Value, TEXT("Water"))) { return EDBAElement::Water; }
			if (StableNameEquals(Value, TEXT("Wood"))) { return EDBAElement::Wood; }
			if (StableNameEquals(Value, TEXT("Gold"))) { return EDBAElement::Gold; }
			if (StableNameEquals(Value, TEXT("Earth"))) { return EDBAElement::Earth; }
			return EDBAElement::None;
		}

		EDBAFiveCamp ParseStableFiveCampName(const FString& Value)
		{
			if (StableNameEquals(Value, TEXT("East"))) { return EDBAFiveCamp::East; }
			if (StableNameEquals(Value, TEXT("West"))) { return EDBAFiveCamp::West; }
			if (StableNameEquals(Value, TEXT("South"))) { return EDBAFiveCamp::South; }
			if (StableNameEquals(Value, TEXT("North"))) { return EDBAFiveCamp::North; }
			if (StableNameEquals(Value, TEXT("Center"))) { return EDBAFiveCamp::Center; }
			return EDBAFiveCamp::None;
		}
	}

	FString ExtractUrlOption(const FString& Options, const FString& Key)
	{
		TArray<FString> Parts;
		Options.ParseIntoArray(Parts, TEXT("?"), true);

		TArray<FString> AmpersandParts;
		for (const FString& Part : Parts)
		{
			TArray<FString> SplitParts;
			Part.ParseIntoArray(SplitParts, TEXT("&"), true);
			AmpersandParts.Append(SplitParts);
		}

		const FString Prefix = Key + TEXT("=");
		for (FString Part : AmpersandParts)
		{
			Part.TrimStartAndEndInline();
			if (Part.StartsWith(Prefix, ESearchCase::IgnoreCase))
			{
				const FString RawValue = Part.RightChop(Prefix.Len()).TrimStartAndEnd();
				return FGenericPlatformHttp::UrlDecode(RawValue);
			}
		}

		return FString();
	}

	bool TryExtractTeamId(const FString& Options, int32& OutTeamId)
	{
		FString RawTeamId = ExtractUrlOption(Options, TEXT("DBATeamId"));
		if (RawTeamId.IsEmpty())
		{
			RawTeamId = ExtractUrlOption(Options, TEXT("TeamId"));
		}

		RawTeamId.TrimStartAndEndInline();
		if (RawTeamId.IsEmpty() || !RawTeamId.IsNumeric())
		{
			OutTeamId = 0;
			return false;
		}

		const int32 ParsedTeamId = FCString::Atoi(*RawTeamId);
		if (ParsedTeamId <= 0)
		{
			OutTeamId = 0;
			return false;
		}

		OutTeamId = ParsedTeamId;
		return true;
	}

	bool TryExtractCharacterBuildSummary(const FString& Options, FDBACharacterBuildSummary& OutSummary)
	{
		const EDBAZodiac Zodiac = ParseStableZodiacName(ExtractUrlOption(Options, TEXT("DBAZodiac")));
		const EDBAElement Element = ParseStableElementName(ExtractUrlOption(Options, TEXT("DBAElement")));
		const EDBAFiveCamp FiveCamp = ParseStableFiveCampName(ExtractUrlOption(Options, TEXT("DBAFiveCamp")));
		const FName FixedSkillGroupId(*ExtractUrlOption(Options, TEXT("DBAFixedSkillGroupId")));
		const FName ExpectedFixedSkillGroupId = DBACharacterBuild::MakeFixedSkillGroupId(Zodiac, Element);

		OutSummary.Zodiac = Zodiac;
		OutSummary.PrimaryElement = Element;
		OutSummary.FiveCamp = FiveCamp;
		OutSummary.FixedSkillGroupId = ExpectedFixedSkillGroupId;

		if (!OutSummary.IsValid())
		{
			return false;
		}

		return FixedSkillGroupId == ExpectedFixedSkillGroupId;
	}
}
