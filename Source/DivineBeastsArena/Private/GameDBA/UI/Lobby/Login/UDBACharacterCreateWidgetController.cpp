// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Lobby/Login/UDBACharacterCreateWidgetController.h"

#include "GameCore/Session/DBALoginFlowSubsystem.h"
#include "GameDBA/UI/DBAGameUIManager.h"

UDBACharacterCreateWidgetController::UDBACharacterCreateWidgetController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDBACharacterCreateWidgetController::SetCharacterName(const FString& InName)
{
	PendingRequest.CharacterName = InName;
}

void UDBACharacterCreateWidgetController::SetZodiac(EDBAZodiac InZodiac)
{
	PendingRequest.Zodiac = InZodiac;
	PendingRequest.DefaultZodiac = InZodiac;
}

void UDBACharacterCreateWidgetController::SetElement(EDBAElement InElement)
{
	PendingRequest.PrimaryElement = InElement;
	PendingRequest.DefaultElement = InElement;
}

void UDBACharacterCreateWidgetController::SetFiveCamp(EDBAFiveCamp InFiveCamp)
{
	PendingRequest.FiveCamp = InFiveCamp;
	PendingRequest.DefaultFiveCamp = InFiveCamp;
}

void UDBACharacterCreateWidgetController::Submit()
{
	if (UDBALoginFlowSubsystem* Flow = GetLoginFlow())
	{
		if (UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
		{
			if (UDBAGameUIManager* UIManager = GameInstance->GetSubsystem<UDBAGameUIManager>())
			{
				UIManager->ShowLobbyLoadingScreen();
			}
		}
		Flow->SubmitCharacterCreation(PendingRequest);
	}
}

UDBALoginFlowSubsystem* UDBACharacterCreateWidgetController::GetLoginFlow() const
{
	return GetWorld() && GetWorld()->GetGameInstance()
		? GetWorld()->GetGameInstance()->GetSubsystem<UDBALoginFlowSubsystem>()
		: nullptr;
}
