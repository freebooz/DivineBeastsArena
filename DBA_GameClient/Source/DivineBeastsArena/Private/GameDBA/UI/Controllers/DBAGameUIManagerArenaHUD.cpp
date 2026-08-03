// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 文件职责：竞技场 HUD 的 C++ 事件转发与角色绑定实现。
- 边界：只将已发生的数据变化转交给 HUD Controller 和 Widget；不承载战斗规则或网络权威逻辑。
*/

#include "GameDBA/UI/Controllers/DBAGameUIManager.h"

#include "GameDBA/Characters/DBAZodiacCharacterBase.h"
#include "GameDBA/UI/Widgets/Arena/UDBAArenaHUDRootWidgetBase.h"
#include "GameDBA/UI/Controllers/Arena/UDBAArenaHUDWidgetController.h"
#include "GameFramework/PlayerController.h"
void UDBAGameUIManager::UpdateArenaHUDPlayerVitals(float CurrentHP, float MaxHP, float CurrentEnergy, float MaxEnergy)
{
	APlayerController* PC = GetArenaHUDLocalPlayerController();
	if (UDBAArenaHUDWidgetController* Controller = EnsureArenaHUDWidgetController(PC))
	{
		Controller->UpdatePlayerHP(CurrentHP, MaxHP);
		Controller->UpdatePlayerEnergy(CurrentEnergy, MaxEnergy);
	}

	if (ArenaHUDWidget && ArenaHUDWidgetController)
	{
		ArenaHUDWidget->SetWidgetController(ArenaHUDWidgetController);
	}
}

void UDBAGameUIManager::UpdateArenaHUDPlayerLevel(int32 Level)
{
	APlayerController* PC = GetArenaHUDLocalPlayerController();
	if (UDBAArenaHUDWidgetController* Controller = EnsureArenaHUDWidgetController(PC))
	{
		Controller->UpdatePlayerLevel(Level);
	}

	if (ArenaHUDWidget && ArenaHUDWidgetController)
	{
		ArenaHUDWidget->SetWidgetController(ArenaHUDWidgetController);
	}
}

void UDBAGameUIManager::UpdateArenaHUDUltimateEnergy(float CurrentEnergy, float MaxEnergy)
{
	APlayerController* PC = GetArenaHUDLocalPlayerController();
	if (UDBAArenaHUDWidgetController* Controller = EnsureArenaHUDWidgetController(PC))
	{
		Controller->UpdateUltimateEnergy(CurrentEnergy, MaxEnergy);
	}

	if (ArenaHUDWidget && ArenaHUDWidgetController)
	{
		ArenaHUDWidget->SetWidgetController(ArenaHUDWidgetController);
	}
}

void UDBAGameUIManager::UpdateArenaHUDCombatState(int32 ChainLevel, int32 ResonanceLevel)
{
	APlayerController* PC = GetArenaHUDLocalPlayerController();
	if (UDBAArenaHUDWidgetController* Controller = EnsureArenaHUDWidgetController(PC))
	{
		Controller->UpdateChainLevel(ChainLevel);
		Controller->UpdateResonanceLevel(ResonanceLevel);
	}

	if (ArenaHUDWidget && ArenaHUDWidgetController)
	{
		ArenaHUDWidget->SetWidgetController(ArenaHUDWidgetController);
	}
}

void UDBAGameUIManager::UpdateArenaHUDMomentum(int32 MomentumLevel, float MomentumProgress)
{
	APlayerController* PC = GetArenaHUDLocalPlayerController();
	if (UDBAArenaHUDWidgetController* Controller = EnsureArenaHUDWidgetController(PC))
	{
		Controller->UpdateMomentum(MomentumLevel, MomentumProgress);
	}

	if (ArenaHUDWidget && ArenaHUDWidgetController)
	{
		ArenaHUDWidget->SetWidgetController(ArenaHUDWidgetController);
	}
}

void UDBAGameUIManager::AddArenaHUDBuff(const FString& BuffId, float Duration)
{
	APlayerController* PC = GetArenaHUDLocalPlayerController();
	if (UDBAArenaHUDWidgetController* Controller = EnsureArenaHUDWidgetController(PC))
	{
		Controller->AddStatusBuff(BuffId, Duration);
	}
}

void UDBAGameUIManager::RemoveArenaHUDBuff(const FString& BuffId)
{
	APlayerController* PC = GetArenaHUDLocalPlayerController();
	if (UDBAArenaHUDWidgetController* Controller = EnsureArenaHUDWidgetController(PC))
	{
		Controller->RemoveStatusBuff(BuffId);
	}
}

void UDBAGameUIManager::ClearArenaHUDBuffs()
{
	APlayerController* PC = GetArenaHUDLocalPlayerController();
	if (UDBAArenaHUDWidgetController* Controller = EnsureArenaHUDWidgetController(PC))
	{
		Controller->ClearStatusBuffs();
	}
}

void UDBAGameUIManager::AddArenaHUDDebuff(const FString& DebuffId, float Duration)
{
	APlayerController* PC = GetArenaHUDLocalPlayerController();
	if (UDBAArenaHUDWidgetController* Controller = EnsureArenaHUDWidgetController(PC))
	{
		Controller->AddStatusDebuff(DebuffId, Duration);
	}
}

void UDBAGameUIManager::RemoveArenaHUDDebuff(const FString& DebuffId)
{
	APlayerController* PC = GetArenaHUDLocalPlayerController();
	if (UDBAArenaHUDWidgetController* Controller = EnsureArenaHUDWidgetController(PC))
	{
		Controller->RemoveStatusDebuff(DebuffId);
	}
}

void UDBAGameUIManager::ClearArenaHUDDebuffs()
{
	APlayerController* PC = GetArenaHUDLocalPlayerController();
	if (UDBAArenaHUDWidgetController* Controller = EnsureArenaHUDWidgetController(PC))
	{
		Controller->ClearStatusDebuffs();
	}
}

void UDBAGameUIManager::AddArenaHUDCCEffect(const FString& CCId, float Duration)
{
	APlayerController* PC = GetArenaHUDLocalPlayerController();
	if (UDBAArenaHUDWidgetController* Controller = EnsureArenaHUDWidgetController(PC))
	{
		Controller->AddStatusCCEffect(CCId, Duration);
	}
}

void UDBAGameUIManager::RemoveArenaHUDCCEffect(const FString& CCId)
{
	APlayerController* PC = GetArenaHUDLocalPlayerController();
	if (UDBAArenaHUDWidgetController* Controller = EnsureArenaHUDWidgetController(PC))
	{
		Controller->RemoveStatusCCEffect(CCId);
	}
}

void UDBAGameUIManager::ClearArenaHUDCCEffects()
{
	APlayerController* PC = GetArenaHUDLocalPlayerController();
	if (UDBAArenaHUDWidgetController* Controller = EnsureArenaHUDWidgetController(PC))
	{
		Controller->ClearStatusCCEffects();
	}
}

void UDBAGameUIManager::ShowArenaHUDCombatAnnouncement(const FText& Text, float Duration)
{
	APlayerController* PC = GetArenaHUDLocalPlayerController();
	if (UDBAArenaHUDWidgetController* Controller = EnsureArenaHUDWidgetController(PC))
	{
		Controller->ShowCombatAnnouncement(Text, Duration);
	}
}

void UDBAGameUIManager::ClearArenaHUDCombatAnnouncement()
{
	APlayerController* PC = GetArenaHUDLocalPlayerController();
	if (UDBAArenaHUDWidgetController* Controller = EnsureArenaHUDWidgetController(PC))
	{
		Controller->ClearCombatAnnouncement();
	}
}

void UDBAGameUIManager::UpdateArenaHUDCriticalStateHints(bool bLowHP, bool bLowEnergy)
{
	APlayerController* PC = GetArenaHUDLocalPlayerController();
	if (UDBAArenaHUDWidgetController* Controller = EnsureArenaHUDWidgetController(PC))
	{
		Controller->UpdateCriticalStateHints(bLowHP, bLowEnergy);
	}
}

void UDBAGameUIManager::UpdateArenaHUDObjective(const FText& ObjectiveText, float Progress)
{
	APlayerController* PC = GetArenaHUDLocalPlayerController();
	if (UDBAArenaHUDWidgetController* Controller = EnsureArenaHUDWidgetController(PC))
	{
		Controller->UpdateArenaObjective(ObjectiveText, Progress);
	}
}

void UDBAGameUIManager::CompleteArenaHUDObjective()
{
	APlayerController* PC = GetArenaHUDLocalPlayerController();
	if (UDBAArenaHUDWidgetController* Controller = EnsureArenaHUDWidgetController(PC))
	{
		Controller->CompleteArenaObjective();
	}
}

void UDBAGameUIManager::AddArenaHUDEventFeedEntry(const FText& Text, float Duration)
{
	APlayerController* PC = GetArenaHUDLocalPlayerController();
	if (UDBAArenaHUDWidgetController* Controller = EnsureArenaHUDWidgetController(PC))
	{
		Controller->AddEventFeedEntry(Text, Duration);
	}
}

void UDBAGameUIManager::ClearArenaHUDEventFeed()
{
	APlayerController* PC = GetArenaHUDLocalPlayerController();
	if (UDBAArenaHUDWidgetController* Controller = EnsureArenaHUDWidgetController(PC))
	{
		Controller->ClearEventFeed();
	}
}

void UDBAGameUIManager::ShowArenaHUDUltimateReadyPrompt()
{
	APlayerController* PC = GetArenaHUDLocalPlayerController();
	if (UDBAArenaHUDWidgetController* Controller = EnsureArenaHUDWidgetController(PC))
	{
		Controller->ShowUltimateReadyPrompt();
	}
}

void UDBAGameUIManager::HideArenaHUDUltimateReadyPrompt()
{
	APlayerController* PC = GetArenaHUDLocalPlayerController();
	if (UDBAArenaHUDWidgetController* Controller = EnsureArenaHUDWidgetController(PC))
	{
		Controller->HideUltimateReadyPrompt();
	}
}

void UDBAGameUIManager::BindArenaHUDToCharacter(ADBAZodiacCharacterBase* Character)
{
	if (!GetArenaHUDLocalPlayerController())
	{
		return;
	}

	ArenaHUDCharacter = Character;

	if (ArenaHUDWidget)
	{
		ArenaHUDWidget->BindArenaHUDToCharacter(Character);
	}
}
