// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/GAS/Cues/DBACue_Snake_Passive.h"

ADBACue_Snake_Passive::ADBACue_Snake_Passive()
{
	LoadSkillData();
}

void ADBACue_Snake_Passive::LoadSkillData()
{
	Super::LoadSkillData();
}

bool ADBACue_Snake_Passive::OnExecuteGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
{
	return Super::OnExecuteGameplayCue(Target, Parameters);
}

void ADBACue_Snake_Passive::OnActiveGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
{
	Super::OnActiveGameplayCue(Target, Parameters);
}

void ADBACue_Snake_Passive::OnRemoveGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
{
	Super::OnRemoveGameplayCue(Target, Parameters);
}
