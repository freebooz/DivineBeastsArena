// Copyright FreeboozStudio. All Rights Reserved.

#include "MobaBase/Data/DBAMobaAbilitySetData.h"

bool UDBAMobaAbilitySetData::IsValid() const
{
	// 至少包含一个 Ability / Effect / AttributeSet
	return Abilities.Num() > 0 || Effects.Num() > 0 || AttributeSets.Num() > 0;
}

FString UDBAMobaAbilitySetData::GetDescription() const
{
	return FString::Printf(TEXT("Abilities: %d, Effects: %d, AttributeSets: %d"),
		Abilities.Num(), Effects.Num(), AttributeSets.Num());
}
