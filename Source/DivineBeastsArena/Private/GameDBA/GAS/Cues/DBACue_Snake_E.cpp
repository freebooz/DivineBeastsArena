// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/GAS/Cues/DBACue_Snake_E.h"

ADBACue_Snake_E::ADBACue_Snake_E()
{
	LoadSkillData();
}

void ADBACue_Snake_E::LoadSkillData()
{
	Super::LoadSkillData();
}

bool ADBACue_Snake_E::OnExecuteGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
{
	return Super::OnExecuteGameplayCue(Target, Parameters);
}

void ADBACue_Snake_E::OnActiveGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
{
	Super::OnActiveGameplayCue(Target, Parameters);
}

void ADBACue_Snake_E::OnRemoveGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
{
	Super::OnRemoveGameplayCue(Target, Parameters);
}
