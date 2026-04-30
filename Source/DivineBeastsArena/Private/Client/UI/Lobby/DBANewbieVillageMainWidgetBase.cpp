// Copyright Epic Games, Inc. All Rights Reserved.

#include "Client/UI/Lobby/DBANewbieVillageMainWidgetBase.h"
#include "Client/UI/Lobby/DBANewbieTaskTrackerWidgetBase.h"

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
		ShowGatePrompt(true, FText::FromString(TEXT("完成新手任务后解锁")));
		return;
	}
}
