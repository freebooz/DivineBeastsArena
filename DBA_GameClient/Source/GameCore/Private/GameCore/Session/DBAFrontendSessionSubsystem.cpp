// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameCore/Session/DBAFrontendSessionSubsystem.h"

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
	TrySetCurrentTravelContext(Context);
}

bool UDBAFrontendSessionSubsystem::TrySetCurrentTravelContext(const FDBATravelContext& Context)
{
	if (!Context.IsValid())
	{
		LogSubsystemWarning(FString::Printf(TEXT("拒绝无效 Travel 上下文：Map=%s Server=%s:%d"),
			*Context.MapName,
			*Context.ServerAddress,
			Context.ServerPort));
		return false;
	}

	if (!Context.HasValidCharacterBuildSummary())
	{
		LogSubsystemWarning(FString::Printf(TEXT("拒绝无效 Travel 构筑身份：生肖=%s 元素=%s 阵营=%s 固定技能组=%s"),
			*Context.SelectedZodiacId.ToString(),
			*Context.SelectedElementId.ToString(),
			*Context.SelectedFiveCampId.ToString(),
			*Context.FixedSkillGroupId.ToString()));
		return false;
	}

	CurrentTravelContext = Context;
	LogSubsystemInfo(FString::Printf(TEXT("设置当前 Travel 上下文：%s"), *Context.MapName));
	SetState(EDBAFrontendSessionState::Loading);
	return true;
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
