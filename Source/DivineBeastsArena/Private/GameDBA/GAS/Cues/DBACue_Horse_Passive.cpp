// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/GAS/Cues/DBACue_Horse_Passive.h"

ADBACue_Horse_Passive::ADBACue_Horse_Passive()
{
	LoadSkillData();
}

void ADBACue_Horse_Passive::LoadSkillData()
{
	Super::LoadSkillData();
}

bool ADBACue_Horse_Passive::OnExecuteGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
{
	return Super::OnExecuteGameplayCue(Target, Parameters);
}

void ADBACue_Horse_Passive::OnActiveGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
{
	Super::OnActiveGameplayCue(Target, Parameters);
}

void ADBACue_Horse_Passive::OnRemoveGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
{
	Super::OnRemoveGameplayCue(Target, Parameters);
}
