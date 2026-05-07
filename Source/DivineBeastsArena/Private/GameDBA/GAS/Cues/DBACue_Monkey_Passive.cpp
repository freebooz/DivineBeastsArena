// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/GAS/Cues/DBACue_Monkey_Passive.h"

ADBACue_Monkey_Passive::ADBACue_Monkey_Passive()
{
	LoadSkillData();
}

void ADBACue_Monkey_Passive::LoadSkillData()
{
	Super::LoadSkillData();
}

bool ADBACue_Monkey_Passive::OnExecuteGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
{
	return Super::OnExecuteGameplayCue(Target, Parameters);
}

void ADBACue_Monkey_Passive::OnActiveGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
{
	Super::OnActiveGameplayCue(Target, Parameters);
}

void ADBACue_Monkey_Passive::OnRemoveGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
{
	Super::OnRemoveGameplayCue(Target, Parameters);
}
