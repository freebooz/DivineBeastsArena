// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Lobby/UDBANewbieVillageMainWidgetBase.h"
#include "GameDBA/UI/Lobby/UDBANewbieTaskTrackerWidgetBase.h"

UDBANewbieVillageMainWidgetBase::UDBANewbieVillageMainWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bIsMainLobbyUnlocked(false)
{
}

void UDBANewbieVillageMainWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UDBANewbieVillageMainWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	RefreshTaskTracker();
}

void UDBANewbieVillageMainWidgetBase::NativeDestruct()
{
	Super::NativeDestruct();
}

void UDBANewbieVillageMainWidgetBase::RefreshTaskTracker()
{
	BP_OnTaskTrackerRefreshed();
}

void UDBANewbieVillageMainWidgetBase::ShowGatePrompt(bool bIsLocked, const FText& UnlockCondition)
{
	BP_OnGatePromptShown(bIsLocked, UnlockCondition);
}

void UDBANewbieVillageMainWidgetBase::SkipNewbieVillage()
{
	bIsMainLobbyUnlocked = true;
	EnterMainLobby();
}

void UDBANewbieVillageMainWidgetBase::EnterMainLobby()
{
	if (!bIsMainLobbyUnlocked)
	{
		ShowGatePrompt(true, FText::GetEmpty());
		return;
	}
}

