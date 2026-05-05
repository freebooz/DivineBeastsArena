// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Lobby/UDBAMainLobbyWidgetBase.h"
#include "GameDBA/UI/Lobby/UDBAMainLobbyWidgetController.h"
#include "GameDBA/UI/Lobby/UDBAPartyPanelWidgetBase.h"
#include "GameDBA/UI/Lobby/UDBAQueueModeSelectWidgetBase.h"

UDBAMainLobbyWidgetBase::UDBAMainLobbyWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CurrentFiveCampTheme(EDBAFiveCamp::Center)
{
}

void UDBAMainLobbyWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	TryBindWidgetController();
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

void UDBAMainLobbyWidgetBase::TryBindWidgetController()
{
	// 自动获取Controller - 子类实现具体逻辑
	// 默认从WidgetTree中查找对应Controller
	if (UObject* Controller = GetWidgetTreeParent.Get())
	{
		SetWidgetController(Cast<UDBAMainLobbyWidgetController>(Controller));
	}
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
