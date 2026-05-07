// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/GAS/Cues/DBACue_Tiger_Q.h"

ADBACue_Tiger_Q::ADBACue_Tiger_Q()
{
	LoadSkillData();
}

void ADBACue_Tiger_Q::LoadSkillData()
{
	Super::LoadSkillData();
}

bool ADBACue_Tiger_Q::OnExecuteGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
{
	return Super::OnExecuteGameplayCue(Target, Parameters);
}

void ADBACue_Tiger_Q::OnActiveGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
{
	Super::OnActiveGameplayCue(Target, Parameters);
}

void ADBACue_Tiger_Q::OnRemoveGameplayCue(AActor* Target, const FGameplayCueParameters& Parameters)
{
	Super::OnRemoveGameplayCue(Target, Parameters);
}
