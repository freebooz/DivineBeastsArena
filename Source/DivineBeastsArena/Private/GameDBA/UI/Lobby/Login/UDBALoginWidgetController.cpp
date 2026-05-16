// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Lobby/Login/UDBALoginWidgetController.h"

UDBALoginWidgetController::UDBALoginWidgetController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDBALoginWidgetController::Start()
{
	if (UDBALoginFlowSubsystem* Flow = GetLoginFlow())
	{
		Flow->OnFlowError.RemoveDynamic(this, &UDBALoginWidgetController::HandleFlowError);
		Flow->OnFlowStateChanged.RemoveDynamic(this, &UDBALoginWidgetController::HandleFlowStateChanged);
		Flow->OnFlowError.AddDynamic(this, &UDBALoginWidgetController::HandleFlowError);
		Flow->OnFlowStateChanged.AddDynamic(this, &UDBALoginWidgetController::HandleFlowStateChanged);
		Flow->StartLoginFlow();
	}
}

void UDBALoginWidgetController::LoginWithEmail(const FString& Email, const FString& Password)
{
	if (UDBALoginFlowSubsystem* Flow = GetLoginFlow())
	{
		Flow->SubmitLogin(Email, Password);
	}
}

void UDBALoginWidgetController::LoginAsGuest()
{
	if (UDBALoginFlowSubsystem* Flow = GetLoginFlow())
	{
		Flow->SubmitGuestLogin();
	}
}

void UDBALoginWidgetController::LoginDebug(const FString& DisplayName)
{
	if (UDBALoginFlowSubsystem* Flow = GetLoginFlow())
	{
		Flow->SubmitDebugLogin(DisplayName);
	}
}

void UDBALoginWidgetController::HandleFlowError(const FString& ErrorMessage)
{
	OnLoginError.Broadcast(ErrorMessage);
}

void UDBALoginWidgetController::HandleFlowStateChanged(EDBALoginFlowState State)
{
	OnLoginStateChanged.Broadcast(State);
}

UDBALoginFlowSubsystem* UDBALoginWidgetController::GetLoginFlow() const
{
	return GetWorld() && GetWorld()->GetGameInstance()
		? GetWorld()->GetGameInstance()->GetSubsystem<UDBALoginFlowSubsystem>()
		: nullptr;
}
