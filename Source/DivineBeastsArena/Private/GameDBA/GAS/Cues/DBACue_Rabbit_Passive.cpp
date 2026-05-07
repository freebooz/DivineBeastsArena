// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/GAS/Cues/DBACue_Rabbit_Passive.h"

ADBACue_Rabbit_Passive::ADBACue_Rabbit_Passive()
{
	LoadSkillData();
}

void ADBACue_Rabbit_Passive::LoadSkillData()
{
	Super::LoadSkillData();
}

bool ADBACue_Rabbit_Passive::OnExecuteGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
{
	return Super::OnExecuteGameplayCue(Target, Parameters);
}

void ADBACue_Rabbit_Passive::OnActiveGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
{
	Super::OnActiveGameplayCue(Target, Parameters);
}

void ADBACue_Rabbit_Passive::OnRemoveGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
{
	Super::OnRemoveGameplayCue(Target, Parameters);
}
