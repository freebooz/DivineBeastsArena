// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/GAS/Cues/DBACue_Dragon_Passive.h"

ADBACue_Dragon_Passive::ADBACue_Dragon_Passive()
{
	LoadSkillData();
}

void ADBACue_Dragon_Passive::LoadSkillData()
{
	Super::LoadSkillData();
}

bool ADBACue_Dragon_Passive::OnExecuteGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
{
	return Super::OnExecuteGameplayCue(Target, Parameters);
}

void ADBACue_Dragon_Passive::OnActiveGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
{
	Super::OnActiveGameplayCue(Target, Parameters);
}

void ADBACue_Dragon_Passive::OnRemoveGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
{
	Super::OnRemoveGameplayCue(Target, Parameters);
}
