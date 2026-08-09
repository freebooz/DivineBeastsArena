// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Frontend/Auth/DBALoginViewModel.h"

void UDBALoginViewModel::SetOperationState(const EDBAAsyncOperationState InOperationState)
{
	if (OperationState != InOperationState)
	{
		OperationState = InOperationState;
		OnChanged.Broadcast();
	}
}

void UDBALoginViewModel::SetLastError(const FDBAApiError& InError)
{
	LastError = InError;
	const EDBAAsyncOperationState NewState = InError.Category == EDBANetworkErrorCategory::Timeout
		? EDBAAsyncOperationState::TimedOut
		: EDBAAsyncOperationState::Failed;
	if (OperationState != NewState)
	{
		OperationState = NewState;
	}
	OnChanged.Broadcast();
}

void UDBALoginViewModel::ClearError()
{
	if (LastError.IsError())
	{
		LastError = FDBAApiError();
		OnChanged.Broadcast();
	}
}

void UDBALoginViewModel::SetGuestLoginEnabled(const bool bEnabled)
{
	if (bGuestLoginEnabled != bEnabled)
	{
		bGuestLoginEnabled = bEnabled;
		OnChanged.Broadcast();
	}
}

void UDBALoginViewModel::SetRememberSession(const bool bRemember)
{
	if (bRememberSession != bRemember)
	{
		bRememberSession = bRemember;
		OnChanged.Broadcast();
	}
}
