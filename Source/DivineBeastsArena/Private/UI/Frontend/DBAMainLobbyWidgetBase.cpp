// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Frontend/DBAMainLobbyWidgetBase.h"
#include "UI/Frontend/DBAMainLobbyWidgetController.h"
#include "UI/Frontend/DBAPartyPanelWidgetBase.h"
#include "UI/Frontend/DBAQueueModeSelectWidgetBase.h"

UDBAMainLobbyWidgetBase::UDBAMainLobbyWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CurrentFiveCampTheme(EDBAFiveCamp::Center)
{
}

void UDBAMainLobbyWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UDBAMainLobbyWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	RefreshPartyInfo();
}

void UDBAMainLobbyWidgetBase::NativeDestruct()
{
	Super::NativeDestruct();
}

void UDBAMainLobbyWidgetBase::SetWidgetController(UDBAMainLobbyWidgetController* InController)
{
	WidgetController = InController;
}

void UDBAMainLobbyWidgetBase::RefreshPartyInfo()
{
	if (WidgetController)
	{
		WidgetController->RequestPartyInfo();
	}

	BP_OnPartyInfoRefreshed();
}

void UDBAMainLobbyWidgetBase::SwitchFiveCampTheme(EDBAFiveCamp FiveCamp)
{
	CurrentFiveCampTheme = FiveCamp;
	BP_OnFiveCampThemeSwitched(FiveCamp);
}

void UDBAMainLobbyWidgetBase::NavigateToNewbieVillage()
{
}

void UDBAMainLobbyWidgetBase::NavigateToPractice()
{
}

void UDBAMainLobbyWidgetBase::OpenQueueModeSelect()
{
	if (QueueModeSelect)
	{
		QueueModeSelect->SetVisibility(ESlateVisibility::Visible);
	}
}

void UDBAMainLobbyWidgetBase::OpenFriendList()
{
}

void UDBAMainLobbyWidgetBase::OpenSettings()
{
}

void UDBAMainLobbyWidgetBase::ExitGame()
{
}
