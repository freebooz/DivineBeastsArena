// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/GAS/Effects/DBAGE_Cooldown.h"

UDBAGE_Cooldown::UDBAGE_Cooldown()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(CooldownDuration));
}

void UDBAGE_Cooldown::PostLoad()
{
	Super::PostLoad();
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(CooldownDuration));
}