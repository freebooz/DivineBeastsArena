// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/DBAGameUIManager.h"
#include "GameDBA/UI/Lobby/UDBAMainLobbyWidgetBase.h"
#include "GameDBA/UI/Arena/UDBAArenaHUDRootWidgetBase.h"
#include "Blueprint/WidgetLayoutBuilder.h"
#include "Components/CanvasPanel.h"

UDBAGameUIManager::UDBAGameUIManager()
	: Super()
{
}

void UDBAGameUIManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UDBAGameUIManager::Deinitialize()
{
	ClearAllUI();
	Super::Deinitialize();
}

void UDBAGameUIManager::ShowMainLobby()
{
	if (!MainLobbyWidget)
	{
		CreateMainLobbyWidget();
	}

	if (MainLobbyWidget && !bMainLobbyVisible)
	{
		MainLobbyWidget->AddToViewport(0);
		MainLobbyWidget->Activate();
		bMainLobbyVisible = true;
	}
}

void UDBAGameUIManager::HideMainLobby()
{
	if (MainLobbyWidget && bMainLobbyVisible)
	{
		MainLobbyWidget->Deactivate();
		MainLobbyWidget->RemoveFromParent();
		bMainLobbyVisible = false;
	}
}

void UDBAGameUIManager::ShowArenaHUD()
{
	if (!ArenaHUDWidget)
	{
		CreateArenaHUDWidget();
	}

	if (ArenaHUDWidget && !bArenaHUDVisible)
	{
		ArenaHUDWidget->AddToViewport(100);
		ArenaHUDWidget->Activate();
		bArenaHUDVisible = true;
	}
}

void UDBAGameUIManager::HideArenaHUD()
{
	if (ArenaHUDWidget && bArenaHUDVisible)
	{
		ArenaHUDWidget->Deactivate();
		ArenaHUDWidget->RemoveFromParent();
		bArenaHUDVisible = false;
	}
}

void UDBAGameUIManager::ClearAllUI()
{
	HideMainLobby();
	HideArenaHUD();
}

void UDBAGameUIManager::CreateMainLobbyWidget()
{
	if (!MainLobbyWidgetClass.IsValid())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			MainLobbyWidget = CreateWidget<UDBAMainLobbyWidgetBase>(PC, MainLobbyWidgetClass);
		}
	}
}

void UDBAGameUIManager::CreateArenaHUDWidget()
{
	if (!ArenaHUDWidgetClass.IsValid())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			ArenaHUDWidget = CreateWidget<UDBAArenaHUDRootWidgetBase>(PC, ArenaHUDWidgetClass);
		}
	}
}