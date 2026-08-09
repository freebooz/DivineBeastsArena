// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 解析 API 响应时复用 GameBackendClient 已有的连接 DTO 解析器，避免重复维护 JoinTicket、角色构筑与 Travel 参数协议。
- 所有回调均以请求代次和 UObject 生命周期为界；旧会话结果不会覆盖新服选择或已关闭页面。
*/

#include "GameDBA/Frontend/Session/DBAGameSessionSubsystem.h"

#include "Dom/JsonObject.h"
#include "GameBackendSessionService.h"
#include "GameBackendClientSettings.h"
#include "GameDBA/Frontend/Online/DBAApiClientSubsystem.h"
#include "GameDBA/Frontend/Online/DBAApiJsonSerializer.h"
#include "GameDBA/Frontend/Session/DBAFrontendTravelCoordinator.h"
#include "GameCore/Core/DBALogChannels.h"
#include "Engine/World.h"
#include "TimerManager.h"

bool UDBAGameSessionSubsystem::IsSupportedInCurrentEnvironment() const
{
	// Dedicated Server 不需要前台 HTTP、预览释放或 ClientTravel 协调。
	return !IsRunningDedicatedServer();
}

void UDBAGameSessionSubsystem::OnSubsystemDeinitialize()
{
	CancelEnterGame();
}

void UDBAGameSessionSubsystem::EnterGame(const FString& CharacterId, const FString& ServerId, FDBAEnterGameCompletion Completion)
{
	if (bEnterGameInFlight)
	{
		if (Completion)
		{
			Completion(FDBAOperationResult::Failure(EDBAErrorCode::InvalidState, TEXT("正在申请进入游戏，请勿重复操作。")));
		}
		return;
	}

	if (CharacterId.TrimStartAndEnd().IsEmpty() || ServerId.TrimStartAndEnd().IsEmpty())
	{
		if (Completion)
		{
			Completion(FDBAOperationResult::Failure(EDBAErrorCode::InvalidData, TEXT("角色或区服信息无效，无法进入游戏。")));
		}
		return;
	}

	bEnterGameInFlight = true;
	PendingCharacterId = CharacterId.TrimStartAndEnd();
	PendingServerId = ServerId.TrimStartAndEnd();
	PendingAttempt = 0;
	PendingCompletion = MoveTemp(Completion);
	++EnterRequestGeneration;
	SendEnterRequest(EnterRequestGeneration);
}

void UDBAGameSessionSubsystem::CancelEnterGame()
{
	++EnterRequestGeneration;
	if (UDBAApiClientSubsystem* ApiClient = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAApiClientSubsystem>() : nullptr)
	{
		if (ActiveApiRequestId.IsValid())
		{
			ApiClient->CancelRequest(ActiveApiRequestId);
		}
	}
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RetryTimerHandle);
	}
	ActiveApiRequestId.Invalidate();
	PendingCharacterId.Reset();
	PendingServerId.Reset();
	PendingAttempt = 0;
	bEnterGameInFlight = false;
	PendingCompletion = nullptr;
}

void UDBAGameSessionSubsystem::SendEnterRequest(const uint64 RequestGeneration)
{
	if (!bEnterGameInFlight || RequestGeneration != EnterRequestGeneration)
	{
		return;
	}

	UDBAApiClientSubsystem* ApiClient = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAApiClientSubsystem>() : nullptr;
	if (!ApiClient)
	{
		CompleteEnter(RequestGeneration, FDBAOperationResult::Failure(EDBAErrorCode::ServiceUnavailable, TEXT("入服网络服务不可用。")));
		return;
	}

	const TSharedRef<FJsonObject> RequestObject = MakeShared<FJsonObject>();
	RequestObject->SetStringField(TEXT("characterId"), PendingCharacterId);
	RequestObject->SetStringField(TEXT("serverId"), PendingServerId);
	FString RequestJson;
	if (!FDBAApiJsonSerializer::SerializeObject(RequestObject, RequestJson))
	{
		CompleteEnter(RequestGeneration, FDBAOperationResult::Failure(EDBAErrorCode::InvalidData, TEXT("进入游戏请求序列化失败。")));
		return;
	}

	FDBAApiRequest Request;
	Request.Verb = EDBAApiHttpVerb::Post;
	Request.Path = TEXT("/api/v1/game/enter");
	Request.JsonBody = MoveTemp(RequestJson);
	Request.bRequiresAuthentication = true;
	Request.CorrelationId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphensLower);
	ActiveApiRequestId = ApiClient->Send(Request, this, [this, RequestGeneration](const FDBAApiResponse& Response)
	{
		HandleEnterResponse(RequestGeneration, Response);
	});
}

void UDBAGameSessionSubsystem::HandleEnterResponse(const uint64 RequestGeneration, const FDBAApiResponse& Response)
{
	if (!bEnterGameInFlight || RequestGeneration != EnterRequestGeneration)
	{
		return;
	}
	ActiveApiRequestId.Invalidate();
	if (!Response.Result.bSuccess)
	{
		CompleteEnter(RequestGeneration, Response.Result);
		return;
	}

	FString Status;
	FDBAEnterWorldConnection Connection;
	if (!ParseEnterResponse(Response.DomainJson, Status, Connection))
	{
		CompleteEnter(RequestGeneration, FDBAOperationResult::Failure(EDBAErrorCode::InvalidData, TEXT("进入游戏响应格式无效。")));
		return;
	}

	if (Status.Equals(TEXT("PENDING"), ESearchCase::IgnoreCase))
	{
		ScheduleRetry(RequestGeneration);
		return;
	}
	if (!Status.Equals(TEXT("READY"), ESearchCase::IgnoreCase) || !Connection.IsValid())
	{
		CompleteEnter(RequestGeneration, FDBAOperationResult::Failure(EDBAErrorCode::InvalidData, TEXT("游戏服务器连接信息不完整。")));
		return;
	}

	UDBAFrontendTravelCoordinator* TravelCoordinator = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAFrontendTravelCoordinator>() : nullptr;
	if (!TravelCoordinator)
	{
		CompleteEnter(RequestGeneration, FDBAOperationResult::Failure(EDBAErrorCode::ServiceUnavailable, TEXT("前台旅行协调服务不可用。")));
		return;
	}

	// 票据只在该短暂调用栈内传给旅行协调器，绝不打印或写入 FrontendSessionContext。
	TravelCoordinator->TravelToGameWorld(Connection, [this, RequestGeneration](const FDBAOperationResult& Result)
	{
		CompleteEnter(RequestGeneration, Result);
	});
}

void UDBAGameSessionSubsystem::ScheduleRetry(const uint64 RequestGeneration)
{
	const UDBAExternalServiceSettings* Settings = GetDefault<UDBAExternalServiceSettings>();
	const int32 MaxAttempts = Settings ? FMath::Max(1, Settings->VillageConnectionMaxAttempts) : 1;
	if (++PendingAttempt >= MaxAttempts)
	{
		CompleteEnter(RequestGeneration, FDBAOperationResult::Failure(EDBAErrorCode::ServiceUnavailable, TEXT("游戏服务器在等待时间内未就绪，请稍后重试。")));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		CompleteEnter(RequestGeneration, FDBAOperationResult::Failure(EDBAErrorCode::ServiceUnavailable, TEXT("当前世界无效，无法继续等待游戏服务器。")));
		return;
	}
	const float Delay = Settings ? FMath::Max(0.1f, Settings->VillageConnectionRetryDelaySeconds) : 1.0f;
	World->GetTimerManager().SetTimer(RetryTimerHandle, FTimerDelegate::CreateWeakLambda(this, [this, RequestGeneration]()
	{
		SendEnterRequest(RequestGeneration);
	}), Delay, false);
}

void UDBAGameSessionSubsystem::CompleteEnter(const uint64 RequestGeneration, const FDBAOperationResult& Result)
{
	if (RequestGeneration != EnterRequestGeneration)
	{
		return;
	}
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RetryTimerHandle);
	}
	ActiveApiRequestId.Invalidate();
	PendingCharacterId.Reset();
	PendingServerId.Reset();
	PendingAttempt = 0;
	bEnterGameInFlight = false;
	FDBAEnterGameCompletion Completion = MoveTemp(PendingCompletion);
	if (Completion)
	{
		Completion(Result);
	}
}

bool UDBAGameSessionSubsystem::ParseEnterResponse(const FString& DomainJson, FString& OutStatus, FDBAEnterWorldConnection& OutConnection) const
{
	OutStatus.Reset();
	OutConnection = FDBAEnterWorldConnection();
	TSharedPtr<FJsonObject> Root;
	if (!FDBAApiJsonSerializer::DeserializeObject(DomainJson, Root) || !Root.IsValid())
	{
		return false;
	}
	const TSharedPtr<FJsonObject>* DataObject = nullptr;
	const TSharedPtr<FJsonObject> Payload = Root->TryGetObjectField(TEXT("data"), DataObject) && DataObject && DataObject->IsValid() ? *DataObject : Root;
	if (!Payload->TryGetStringField(TEXT("status"), OutStatus))
	{
		return false;
	}
	if (OutStatus.Equals(TEXT("PENDING"), ESearchCase::IgnoreCase))
	{
		return true;
	}

	FDBA_GameBackendSessionConnection ParsedConnection;
	if (!UDBA_GameBackendSessionService::ParseConnectionData(DomainJson, ParsedConnection))
	{
		return false;
	}
	OutConnection.SessionId = ParsedConnection.SessionId;
	OutConnection.ServerHost = ParsedConnection.Ip;
	OutConnection.ServerPort = ParsedConnection.Port;
	OutConnection.GameTicket = ParsedConnection.JoinTicket;
	OutConnection.PlayerId = ParsedConnection.PlayerId;
	OutConnection.CharacterId = ParsedConnection.CharacterId;
	OutConnection.ServerInstanceId = ParsedConnection.ServerInstanceId;
	OutConnection.BuildId = ParsedConnection.BuildId;
	OutConnection.TeamId = ParsedConnection.TeamId;
	OutConnection.Zodiac = ParsedConnection.Zodiac;
	OutConnection.PrimaryElement = ParsedConnection.PrimaryElement;
	OutConnection.FiveCamp = ParsedConnection.FiveCamp;
	OutConnection.FixedSkillGroupId = ParsedConnection.FixedSkillGroupId;
	return true;
}
