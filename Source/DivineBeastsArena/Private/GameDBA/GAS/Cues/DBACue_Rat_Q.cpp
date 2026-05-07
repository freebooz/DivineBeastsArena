// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/GAS/Cues/DBACue_Rat_Q.h"

ADBACue_Rat_Q::ADBACue_Rat_Q()
{
	LoadSkillData();
}

void ADBACue_Rat_Q::LoadSkillData()
{
	Super::LoadSkillData();
}

bool ADBACue_Rat_Q::OnExecuteGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
{
	return Super::OnExecuteGameplayCue(Target, Parameters);
}

void ADBACue_Rat_Q::OnActiveGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
{
	Super::OnActiveGameplayCue(Target, Parameters);
}

void ADBACue_Rat_Q::OnRemoveGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
{
	Super::OnRemoveGameplayCue(Target, Parameters);
}
