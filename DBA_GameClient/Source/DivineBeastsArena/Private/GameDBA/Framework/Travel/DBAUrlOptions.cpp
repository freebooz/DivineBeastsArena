// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Framework/Travel/DBAUrlOptions.h"

#include "GenericPlatform/GenericPlatformHttp.h"

namespace DBAUrlOptions
{
	namespace
	{
		FName ExtractBuildIdentityOption(const FString& Options, const TCHAR* Key)
		{
			const FString Value = ExtractUrlOption(Options, Key);
			return Value.IsEmpty() ? NAME_None : FName(*Value);
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
		OutSummary = DBACharacterBuild::MakeBuildSummary(
			ExtractBuildIdentityOption(Options, TEXT("DBAZodiac")),
			ExtractBuildIdentityOption(Options, TEXT("DBAElement")),
			ExtractBuildIdentityOption(Options, TEXT("DBAFiveCamp")),
			ExtractBuildIdentityOption(Options, TEXT("DBAFixedSkillGroupId")));

		// 只在此处解析中性 URL 传输契约。固定技能组关系由 Arena 数据资产负责校验。
		return OutSummary.IsValid();
	}
}
