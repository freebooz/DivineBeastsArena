// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Lobby/Login/UDBACharacterSelectWidgetController.h"

#include "GameCore/Session/DBALoginFlowSubsystem.h"

UDBACharacterSelectWidgetController::UDBACharacterSelectWidgetController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDBACharacterSelectWidgetController::BindLoginFlow()
{
	if (UDBALoginFlowSubsystem* Flow = GetLoginFlow())
	{
		Flow->OnCharactersLoaded.RemoveDynamic(this, &UDBACharacterSelectWidgetController::HandleCharactersLoaded);
		Flow->OnCharactersLoaded.AddDynamic(this, &UDBACharacterSelectWidgetController::HandleCharactersLoaded);
		OnCharactersChanged.Broadcast(Flow->GetCachedCharacters());
	}
}

void UDBACharacterSelectWidgetController::SelectCharacter(const FDBACharacterId& CharacterId)
{
	if (UDBALoginFlowSubsystem* Flow = GetLoginFlow())
	{
		Flow->SubmitCharacterSelection(CharacterId);
	}
}

void UDBACharacterSelectWidgetController::HandleCharactersLoaded(const TArray<FDBACharacterSummary>& Characters)
{
	OnCharactersChanged.Broadcast(Characters);
}

UDBALoginFlowSubsystem* UDBACharacterSelectWidgetController::GetLoginFlow() const
{
	return GetWorld() && GetWorld()->GetGameInstance()
		? GetWorld()->GetGameInstance()->GetSubsystem<UDBALoginFlowSubsystem>()
		: nullptr;
}
