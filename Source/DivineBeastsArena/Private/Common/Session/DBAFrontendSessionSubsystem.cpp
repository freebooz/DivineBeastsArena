// Copyright FreeboozStudio. All Rights Reserved.

#include "Common/Session/DBAFrontendSessionSubsystem.h"

UDBAFrontendSessionSubsystem::UDBAFrontendSessionSubsystem()
{
}

void UDBAFrontendSessionSubsystem::OnSubsystemInitialize()
{
	Super::OnSubsystemInitialize();

	LogSubsystemInfo(TEXT("前台会话子系统初始化"));

	CurrentState = EDBAFrontendSessionState::None;

	bIsInitialized = true;
}

void UDBAFrontendSessionSubsystem::OnSubsystemDeinitialize()
{
	LogSubsystemInfo(TEXT("前台会话子系统反初始化"));

	ResetSession();

	Super::OnSubsystemDeinitialize();
}

bool UDBAFrontendSessionSubsystem::IsSupportedInCurrentEnvironment() const
{
	// Dedicated Server 不需要前台会话子系统
	if (GetGameInstance() && GetGameInstance()->IsDedicatedServerInstance())
	{
		return false;
	}

	return true;
}

void UDBAFrontendSessionSubsystem::SetState(EDBAFrontendSessionState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

	const EDBAFrontendSessionState OldState = CurrentState;
	CurrentState = NewState;

	LogSubsystemInfo(FString::Printf(TEXT("会话状态变化：%d -> %d"), static_cast<uint8>(OldState), static_cast<uint8>(NewState)));

	OnSessionStateChanged.Broadcast(OldState, NewState);
}

void UDBAFrontendSessionSubsystem::SetCurrentPartyInfo(const FDBAPartyInfo& PartyInfo)
{
	CurrentPartyInfo = PartyInfo;
	LogSubsystemInfo(FString::Printf(TEXT("设置当前 Party 信息：%s"), *PartyInfo.PartyId.ToString()));
}

void UDBAFrontendSessionSubsystem::ClearCurrentPartyInfo()
{
	CurrentPartyInfo = FDBAPartyInfo();
	LogSubsystemInfo(TEXT("清空当前 Party 信息"));
}

void UDBAFrontendSessionSubsystem::SetCurrentQueueInfo(const FDBAQueueInfo& QueueInfo)
{
	CurrentQueueInfo = QueueInfo;
	LogSubsystemInfo(FString::Printf(TEXT("设置当前 Queue 信息：%s"), *QueueInfo.QueueId.ToString()));
}

void UDBAFrontendSessionSubsystem::ClearCurrentQueueInfo()
{
	CurrentQueueInfo = FDBAQueueInfo();
	LogSubsystemInfo(TEXT("清空当前 Queue 信息"));
}

void UDBAFrontendSessionSubsystem::SetCurrentReadyCheckInfo(const FDBAReadyCheckInfo& ReadyCheckInfo)
{
	CurrentReadyCheckInfo = ReadyCheckInfo;
	LogSubsystemInfo(FString::Printf(TEXT("设置当前 ReadyCheck 信息：%s"), *ReadyCheckInfo.ReadyCheckId.ToString()));
}

void UDBAFrontendSessionSubsystem::ClearCurrentReadyCheckInfo()
{
	CurrentReadyCheckInfo = FDBAReadyCheckInfo();
	LogSubsystemInfo(TEXT("清空当前 ReadyCheck 信息"));
}

void UDBAFrontendSessionSubsystem::SetCurrentMatchSessionInfo(const FDBAMatchSessionInfo& SessionInfo)
{
	CurrentMatchSessionInfo = SessionInfo;
	LogSubsystemInfo(FString::Printf(TEXT("设置当前 MatchSession 信息：%s"), *SessionInfo.SessionId.ToString()));
}

void UDBAFrontendSessionSubsystem::ClearCurrentMatchSessionInfo()
{
	CurrentMatchSessionInfo = FDBAMatchSessionInfo();
	LogSubsystemInfo(TEXT("清空当前 MatchSession 信息"));
}

void UDBAFrontendSessionSubsystem::SetCurrentTravelContext(const FDBATravelContext& Context)
{
	CurrentTravelContext = Context;
	LogSubsystemInfo(FString::Printf(TEXT("设置当前 Travel 上下文：%s"), *Context.MapName));
}

void UDBAFrontendSessionSubsystem::ClearCurrentTravelContext()
{
	CurrentTravelContext = FDBATravelContext();
	LogSubsystemInfo(TEXT("清空当前 Travel 上下文"));
}

void UDBAFrontendSessionSubsystem::ResetSession()
{
	LogSubsystemInfo(TEXT("重置会话"));

	ClearCurrentPartyInfo();
	ClearCurrentQueueInfo();
	ClearCurrentReadyCheckInfo();
	ClearCurrentMatchSessionInfo();
	ClearCurrentTravelContext();

	SetState(EDBAFrontendSessionState::MainLobby);
}
