// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/GAS/Cues/DBACue_Goat_Passive.h"

ADBACue_Goat_Passive::ADBACue_Goat_Passive()
{
	LoadSkillData();
}

void ADBACue_Goat_Passive::LoadSkillData()
{
	Super::LoadSkillData();
}

bool ADBACue_Goat_Passive::OnExecuteGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
{
	return Super::OnExecuteGameplayCue(Target, Parameters);
}

void ADBACue_Goat_Passive::OnActiveGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
{
	Super::OnActiveGameplayCue(Target, Parameters);
}

void ADBACue_Goat_Passive::OnRemoveGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
{
	Super::OnRemoveGameplayCue(Target, Parameters);
}
