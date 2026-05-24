// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Combat/DBAPlayableSkillCatalogDataAsset.h"

bool UDBAPlayableSkillCatalogDataAsset::GetSkillSpec(int32 SkillSlot, FDBAPlayableSkillRuntimeSpec& OutSpec) const
{
	const TArray<FDBAPlayableSkillRuntimeSpec> NormalizedSpecs = GetAllSkillSpecs();
	for (const FDBAPlayableSkillRuntimeSpec& Spec : NormalizedSpecs)
	{
		if (Spec.SkillSlot == SkillSlot)
		{
			OutSpec = Spec;
			return true;
		}
	}

	return false;
}

TArray<FDBAPlayableSkillRuntimeSpec> UDBAPlayableSkillCatalogDataAsset::GetAllSkillSpecs() const
{
	TArray<FDBAPlayableSkillRuntimeSpec> NormalizedSpecs;
	NormalizedSpecs.Reserve(SkillSpecs.Num());

	for (const FDBAPlayableSkillRuntimeSpec& Spec : SkillSpecs)
	{
		if (Spec.SkillSlot <= 0)
		{
			continue;
		}

		if (FDBAPlayableSkillRuntimeSpec* ExistingSpec = NormalizedSpecs.FindByPredicate([&Spec](const FDBAPlayableSkillRuntimeSpec& Candidate)
			{
				return Candidate.SkillSlot == Spec.SkillSlot;
			}))
		{
			*ExistingSpec = Spec;
		}
		else
		{
			NormalizedSpecs.Add(Spec);
		}
	}

	NormalizedSpecs.Sort([](const FDBAPlayableSkillRuntimeSpec& Left, const FDBAPlayableSkillRuntimeSpec& Right)
	{
		return Left.SkillSlot < Right.SkillSlot;
	});

	return NormalizedSpecs;
}
