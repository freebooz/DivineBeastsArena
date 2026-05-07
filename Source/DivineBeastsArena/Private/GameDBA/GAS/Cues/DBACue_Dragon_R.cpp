// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/GAS/Cues/DBACue_Dragon_R.h"

ADBACue_Dragon_R::ADBACue_Dragon_R()
{
	LoadSkillData();
}

void ADBACue_Dragon_R::LoadSkillData()
{
	Super::LoadSkillData();
}

bool ADBACue_Dragon_R::OnExecuteGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
{
	return Super::OnExecuteGameplayCue(Target, Parameters);
}

void ADBACue_Dragon_R::OnActiveGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
{
	Super::OnActiveGameplayCue(Target, Parameters);
}

void ADBACue_Dragon_R::OnRemoveGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
{
	Super::OnRemoveGameplayCue(Target, Parameters);
}
