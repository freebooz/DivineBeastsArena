// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Character/Appearance/DBAAppearanceCatalogDataAsset.h"

const FDBAAppearanceOptionDefinition* UDBAAppearanceCatalogDataAsset::FindOption(const FName OptionId) const
{
	return Options.FindByPredicate([OptionId](const FDBAAppearanceOptionDefinition& Definition)
	{
		return Definition.OptionId == OptionId;
	});
}

const FDBAAppearanceOptionDefinition* UDBAAppearanceCatalogDataAsset::FindFallback(const EDBAAppearanceSlot Slot, const EDBAZodiac Zodiac) const
{
	return Options.FindByPredicate([this, Slot, Zodiac](const FDBAAppearanceOptionDefinition& Definition)
	{
		return Definition.Slot == Slot && Definition.bFallbackForSlot && IsOptionAllowed(Definition, Zodiac);
	});
}

bool UDBAAppearanceCatalogDataAsset::IsOptionAllowed(const FDBAAppearanceOptionDefinition& Definition, const EDBAZodiac Zodiac) const
{
	return Zodiac != EDBAZodiac::None
		&& (Definition.AllowedZodiacs.IsEmpty() || Definition.AllowedZodiacs.Contains(Zodiac));
}

void UDBAAppearanceCatalogDataAsset::GetAvailableOptionIds(const EDBAZodiac Zodiac, const EDBAAppearanceSlot Slot, TArray<FName>& OutOptionIds) const
{
	OutOptionIds.Reset();
	for (const FDBAAppearanceOptionDefinition& Definition : Options)
	{
		if (Definition.Slot == Slot && !Definition.OptionId.IsNone() && IsOptionAllowed(Definition, Zodiac))
		{
			OutOptionIds.Add(Definition.OptionId);
		}
	}
}

#if WITH_EDITOR
EDataValidationResult UDBAAppearanceCatalogDataAsset::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);
	TSet<FName> OptionIds;
	for (const FDBAAppearanceOptionDefinition& Definition : Options)
	{
		if (Definition.OptionId.IsNone())
		{
			Context.AddError(FText::FromString(TEXT("外观目录存在空 OptionId。")));
			Result = EDataValidationResult::Invalid;
			continue;
		}
		if (OptionIds.Contains(Definition.OptionId))
		{
			Context.AddError(FText::FromString(FString::Printf(TEXT("外观目录存在重复 OptionId：%s。"), *Definition.OptionId.ToString())));
			Result = EDataValidationResult::Invalid;
			continue;
		}
		OptionIds.Add(Definition.OptionId);
		if (Definition.bUseLeaderPose && !Definition.SkeletalMesh.IsNull() && !Definition.CopyPoseAnimationClass.IsNull())
		{
			Context.AddWarning(FText::FromString(FString::Printf(TEXT("外观选项 %s 同时配置 LeaderPose 和 CopyPose；运行时仅在骨架不匹配时使用 CopyPose。"), *Definition.OptionId.ToString())));
		}
	}
	return Result;
}
#endif
