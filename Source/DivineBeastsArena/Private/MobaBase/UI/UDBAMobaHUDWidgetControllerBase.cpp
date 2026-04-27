// Copyright FreeboozStudio. All Rights Reserved.

#include "MobaBase/UI/UDBAMobaHUDWidgetControllerBase.h"

UDBAMobaHUDWidgetControllerBase::UDBAMobaHUDWidgetControllerBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDBAMobaHUDWidgetControllerBase::UpdatePlayerHP(float CurrentHP, float MaxHP)
{
	OnPlayerHPChanged.Broadcast(CurrentHP, MaxHP);
}

void UDBAMobaHUDWidgetControllerBase::UpdatePlayerMana(float CurrentMana, float MaxMana)
{
	OnPlayerManaChanged.Broadcast(CurrentMana, MaxMana);
}

void UDBAMobaHUDWidgetControllerBase::UpdatePlayerXP(float CurrentXP, float XPGain)
{
	OnPlayerXPChanged.Broadcast(CurrentXP);
}

void UDBAMobaHUDWidgetControllerBase::UpdatePlayerGold(int32 CurrentGold)
{
	OnPlayerGoldChanged.Broadcast(CurrentGold);
}

void UDBAMobaHUDWidgetControllerBase::UpdatePlayerStats(int32 Kills, int32 Deaths, int32 Assists)
{
	OnPlayerStatsChanged.Broadcast(Kills, Deaths, Assists);
}

void UDBAMobaHUDWidgetControllerBase::UpdateRespawnTime(float RemainingTime)
{
	OnRespawnTimeChanged.Broadcast(RemainingTime);
}
