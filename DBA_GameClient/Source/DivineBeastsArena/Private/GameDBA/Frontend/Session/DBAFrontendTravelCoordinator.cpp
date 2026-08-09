// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 本实现仅在客户端运行；Dedicated Server 不创建 UI Root、不加载预览，也不执行 ClientTravel。
- Loading 使用 UILayerManager 的请求令牌，嵌套请求不会提前关闭其他系统的全局加载层。
*/

#include "GameDBA/Frontend/Session/DBAFrontendTravelCoordinator.h"

#include "GameBackendSessionService.h"
#include "GameDBA/Frontend/Preview/DBACharacterPreviewSubsystem.h"
#include "GameDBA/Frontend/Session/DBAGameSessionSubsystem.h"
#include "GameDBA/UI/Subsystems/DBAUILayerManagerSubsystem.h"
#include "GameCore/Core/DBALogChannels.h"
#include "GameCore/Networking/Travel/DBATravelSubsystem.h"
#include "UObject/UObjectGlobals.h"

bool UDBAFrontendTravelCoordinator::IsSupportedInCurrentEnvironment() const
{
	return !IsRunningDedicatedServer();
}

void UDBAFrontendTravelCoordinator::OnSubsystemDeinitialize()
{
	CancelPendingTravel();
}

void UDBAFrontendTravelCoordinator::TravelToGameWorld(const FDBAEnterWorldConnection& Connection, FDBAFrontendTravelCompletion Completion)
{
	if (bTravelInFlight)
	{
		if (Completion)
		{
			Completion(FDBAOperationResult::Failure(EDBAErrorCode::InvalidState, TEXT("客户端正在进入游戏世界。")));
		}
		return;
	}
	FString TravelUrl;
	if (!BuildTravelUrl(Connection, TravelUrl))
	{
		if (Completion)
		{
			Completion(FDBAOperationResult::Failure(EDBAErrorCode::InvalidData, TEXT("游戏服务器连接信息无效。")));
		}
		return;
	}

	bTravelInFlight = true;
	// 先切换为不可交互的全局 Loading，再释放只属于前台的预览资源；Travel 成功后旧前台世界会整体卸载。
	if (UDBAUILayerManagerSubsystem* Layers = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAUILayerManagerSubsystem>() : nullptr)
	{
		LoadingRequestToken = Layers->BeginGlobalLoading(FText::FromString(TEXT("正在进入游戏世界…")));
	}
	if (UDBACharacterPreviewSubsystem* Preview = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBACharacterPreviewSubsystem>() : nullptr)
	{
		Preview->ReleasePreview();
	}

	// GameInstanceSubsystem 会跨地图存活；必须在发起 ClientTravel 前订阅地图加载完成事件。
	// 某些本地地图或已缓存地图可以在 RequestClientTravel 返回前完成加载，若晚注册就会遗漏
	// 回调并遗留 Loading 请求令牌，使回退到角色选择页后仍显示不可交互的加载层。
	PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UDBAFrontendTravelCoordinator::HandlePostLoadMap);

	FString TravelError;
	UDBATravelSubsystem* TravelSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBATravelSubsystem>() : nullptr;
	if (!TravelSubsystem || !TravelSubsystem->RequestClientTravel(TravelUrl, TravelError))
	{
		// 尚未进入引擎 Travel 时可以完整撤销本次协调状态；同时移除刚注册的回调，
		// 防止之后其他地图加载错误地结束本次失败请求的 Loading。
		CancelPendingTravel();
		if (Completion)
		{
			Completion(FDBAOperationResult::Failure(EDBAErrorCode::ServiceUnavailable, TravelError.IsEmpty() ? TEXT("客户端旅行启动失败。") : TravelError));
		}
		return;
	}

	// ClientTravel 已由引擎接管。成功结果在此刻交给 Flow；不记录 TravelUrl 或票据，防止敏感参数进入日志。
	if (Completion)
	{
		Completion(FDBAOperationResult::Success());
	}
}

void UDBAFrontendTravelCoordinator::CancelPendingTravel()
{
	// 引擎已开始 ClientTravel 后不能安全地“撤销旅行”；这里只清理尚未开始时的前台 Loading 令牌。
	bTravelInFlight = false;
	if (PostLoadMapHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
		PostLoadMapHandle.Reset();
	}
	EndLoading();
}

bool UDBAFrontendTravelCoordinator::BuildTravelUrl(const FDBAEnterWorldConnection& Connection, FString& OutTravelUrl) const
{
	OutTravelUrl.Reset();
	if (!Connection.IsValid())
	{
		return false;
	}
	// 重用既有协议构造器，确保运行时校验所需的角色、构筑、TeamId 参数与旧 DS 契约保持兼容。
	OutTravelUrl = UDBA_GameBackendSessionService::BuildTravelUrl(
		Connection.ServerHost,
		Connection.ServerPort,
		Connection.SessionId,
		Connection.GameTicket,
		Connection.PlayerId,
		Connection.CharacterId,
		Connection.TeamId,
		Connection.Zodiac,
		Connection.PrimaryElement,
		Connection.FiveCamp,
		Connection.FixedSkillGroupId);
	return !OutTravelUrl.IsEmpty();
}

void UDBAFrontendTravelCoordinator::EndLoading()
{
	if (LoadingRequestToken.IsNone())
	{
		return;
	}
	if (UDBAUILayerManagerSubsystem* Layers = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAUILayerManagerSubsystem>() : nullptr)
	{
		Layers->EndGlobalLoading(LoadingRequestToken);
	}
	LoadingRequestToken = NAME_None;
}

void UDBAFrontendTravelCoordinator::HandlePostLoadMap(UWorld* LoadedWorld)
{
	// 新世界已建立，前台 Loading 根节点无需继续保留；不触碰游戏世界的 HUD 或战斗 UI。
	if (PostLoadMapHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
		PostLoadMapHandle.Reset();
	}
	bTravelInFlight = false;
	EndLoading();
}
