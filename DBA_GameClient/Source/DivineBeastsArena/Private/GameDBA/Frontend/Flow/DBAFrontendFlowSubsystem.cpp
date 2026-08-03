// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Frontend/Flow/DBAFrontendFlowSubsystem.h"

#include "GameDBA/Frontend/Account/DBAOnlineAccountService.h"
#include "GameCore/Core/DBALogChannels.h"
#include "GameBackendClientSettings.h"
#include "GameBackendSessionService.h"
#include "GameDBA/Frontend/Backend/DBABackendFacadeSubsystem.h"
#include "Dom/JsonObject.h"
#include "Engine/World.h"
#include "GameCore/Networking/Travel/DBATravelSubsystem.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "TimerManager.h"

namespace
{
	bool ParseVillageAllocation(const FString& DataJson, FString& OutSessionId)
	{
		TSharedPtr<FJsonObject> Root;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(DataJson);
		if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
		{
			return false;
		}

		TSharedPtr<FJsonObject> Payload = Root;
		const TSharedPtr<FJsonObject>* DataObject = nullptr;
		if (Root->TryGetObjectField(TEXT("data"), DataObject) && DataObject && DataObject->IsValid())
		{
			Payload = *DataObject;
		}
		Payload->TryGetStringField(TEXT("sessionId"), OutSessionId);
		return !OutSessionId.IsEmpty();
	}
}

bool UDBAFrontendFlowSubsystem::ShouldEnterCharacterCreate(int32 CharacterCount)
{
	return CharacterCount <= 0;
}

void UDBAFrontendFlowSubsystem::SetFlowState(EDBALoginFlowState NewState)
{
	if (FlowState == NewState)
	{
		return;
	}

	UE_LOG(LogDBACore, Log, TEXT("[DBAFrontendFlowSubsystem] 流程状态切换：%d -> %d"), static_cast<int32>(FlowState), static_cast<int32>(NewState));
	FlowState = NewState;
	OnFlowStateChanged.Broadcast(NewState);
}

uint64 UDBAFrontendFlowSubsystem::BeginFlowRequest()
{
	return ++FlowRequestGeneration;
}

void UDBAFrontendFlowSubsystem::InvalidatePendingFlowRequests()
{
	++FlowRequestGeneration;
}

bool UDBAFrontendFlowSubsystem::IsFlowRequestCurrent(uint64 RequestGeneration) const
{
	return RequestGeneration == FlowRequestGeneration;
}

void UDBAFrontendFlowSubsystem::BroadcastErrorAndSetState(const FString& ErrorMessage, EDBALoginFlowState NewState)
{
	OnFlowError.Broadcast(ErrorMessage);
	SetFlowState(NewState);
}

void UDBAFrontendFlowSubsystem::StartLoginFlow()
{
	InvalidatePendingFlowRequests();
	CachedCharacters.Reset();
	PendingVillageRequestGeneration = 0;
	PendingVillageSessionId.Reset();
	VillageConnectionAttempt = 0;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(VillageConnectionRetryTimerHandle);
	}
	// 进入前端后先展示登录页，认证只由用户明确操作触发。
	SetFlowState(EDBALoginFlowState::AwaitingLogin);
}

void UDBAFrontendFlowSubsystem::SubmitLogin(const FString& Email, const FString& Password)
{
	const uint64 RequestGeneration = BeginFlowRequest();
	SetFlowState(EDBALoginFlowState::Authenticating);

	UDBAOnlineAccountService* AccountService = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAOnlineAccountService>() : nullptr;
	if (!AccountService)
	{
		BroadcastErrorAndSetState(TEXT("账号服务不可用。"), EDBALoginFlowState::AwaitingLogin);
		return;
	}

	FDBALoginRequest Request;
	Request.LoginType = EDBALoginType::Email;
	Request.Email = Email;
	Request.Password = Password;

	AccountService->Login(Request, FDBAOnLoginComplete::CreateWeakLambda(this, [this, RequestGeneration](const FDBALoginResponse& Response)
	{
		if (!IsFlowRequestCurrent(RequestGeneration))
		{
			UE_LOG(LogDBACore, Verbose, TEXT("[DBAFrontendFlowSubsystem] 忽略已过期的账号登录回调。"));
			return;
		}

		if (Response.bSuccess)
		{
			LoadCharactersAfterLogin();
			return;
		}

		BroadcastErrorAndSetState(Response.ErrorMessage, EDBALoginFlowState::AwaitingLogin);
	}));
}

void UDBAFrontendFlowSubsystem::SubmitGuestLogin()
{
	const uint64 RequestGeneration = BeginFlowRequest();
	SetFlowState(EDBALoginFlowState::Authenticating);

	UDBAOnlineAccountService* AccountService = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAOnlineAccountService>() : nullptr;
	if (!AccountService)
	{
		BroadcastErrorAndSetState(TEXT("账号服务不可用。"), EDBALoginFlowState::AwaitingLogin);
		return;
	}

	AccountService->GuestLogin(FDBAOnLoginComplete::CreateWeakLambda(this, [this, RequestGeneration](const FDBALoginResponse& Response)
	{
		if (!IsFlowRequestCurrent(RequestGeneration))
		{
			UE_LOG(LogDBACore, Verbose, TEXT("[DBAFrontendFlowSubsystem] 忽略已过期的游客登录回调。"));
			return;
		}

		if (Response.bSuccess)
		{
			LoadCharactersAfterLogin();
			return;
		}

		BroadcastErrorAndSetState(Response.ErrorMessage, EDBALoginFlowState::AwaitingLogin);
	}));
}

void UDBAFrontendFlowSubsystem::LoadCharactersAfterLogin()
{
	if (!SynchronizeBackendAuthentication())
	{
		BroadcastErrorAndSetState(TEXT("后端认证上下文同步失败。"), EDBALoginFlowState::AwaitingLogin);
		return;
	}

	const uint64 RequestGeneration = BeginFlowRequest();
	const EDBALoginFlowState PreviousState = FlowState;
	SetFlowState(EDBALoginFlowState::LoadingCharacters);

	UDBAOnlineAccountService* AccountService = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAOnlineAccountService>() : nullptr;
	if (!AccountService)
	{
		BroadcastErrorAndSetState(TEXT("账号服务不可用。"), EDBALoginFlowState::AwaitingLogin);
		return;
	}

	AccountService->GetCharacterList(FDBAOnCharacterListLoaded::CreateWeakLambda(this, [this, PreviousState, RequestGeneration](bool bSuccess, const TArray<FDBACharacterSummary>& Characters)
	{
		if (!IsFlowRequestCurrent(RequestGeneration))
		{
			UE_LOG(LogDBACore, Verbose, TEXT("[DBAFrontendFlowSubsystem] 忽略已过期的角色列表回调。"));
			return;
		}

		if (!bSuccess)
		{
			const EDBALoginFlowState ErrorState =
				(PreviousState == EDBALoginFlowState::CharacterSelecting || PreviousState == EDBALoginFlowState::CharacterCreating)
					? PreviousState
					: EDBALoginFlowState::AwaitingLogin;
			UE_LOG(LogDBACore, Warning, TEXT("[DBAFrontendFlowSubsystem] 角色列表加载失败，不进入创建流程。"));
			BroadcastErrorAndSetState(TEXT("获取角色列表失败。"), ErrorState);
			return;
		}

		CachedCharacters = Characters;
		OnCharactersLoaded.Broadcast(CachedCharacters);
		UE_LOG(LogDBACore, Log, TEXT("[DBAFrontendFlowSubsystem] 角色列表加载完成：%d"), CachedCharacters.Num());
		SetFlowState(ShouldEnterCharacterCreate(CachedCharacters.Num()) ? EDBALoginFlowState::CharacterCreating : EDBALoginFlowState::CharacterSelecting);
	}));
}

bool UDBAFrontendFlowSubsystem::SynchronizeBackendAuthentication()
{
	UDBAOnlineAccountService* AccountService = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UDBAOnlineAccountService>()
		: nullptr;
	UDBABackendFacadeSubsystem* BackendFacade = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UDBABackendFacadeSubsystem>()
		: nullptr;
	FString ErrorMessage;
	if (!BackendFacade || !BackendFacade->SynchronizeAuthentication(AccountService, ErrorMessage))
	{
		return false;
	}
	return true;
}

void UDBAFrontendFlowSubsystem::SubmitCharacterSelection(const FDBACharacterId& CharacterId)
{
	if (FlowState != EDBALoginFlowState::CharacterSelecting && FlowState != EDBALoginFlowState::CharacterCreating)
	{
		BroadcastErrorAndSetState(TEXT("当前状态不允许选择角色。"), EDBALoginFlowState::AwaitingLogin);
		return;
	}
	if (!CharacterId.IsValid() || !CachedCharacters.ContainsByPredicate([&CharacterId](const FDBACharacterSummary& Character)
		{
			return Character.CharacterId == CharacterId;
		}))
	{
		BroadcastErrorAndSetState(TEXT("请选择有效角色。"), FlowState);
		return;
	}

	const uint64 RequestGeneration = BeginFlowRequest();

	UDBAOnlineAccountService* AccountService = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAOnlineAccountService>() : nullptr;
	if (!AccountService)
	{
		BroadcastErrorAndSetState(TEXT("账号服务不可用。"), EDBALoginFlowState::CharacterSelecting);
		return;
	}

	AccountService->SelectCharacter(CharacterId, FDBAOnCharacterSelected::CreateWeakLambda(this, [this, RequestGeneration](const FDBACharacterId& SelectedId)
	{
		if (!IsFlowRequestCurrent(RequestGeneration))
		{
			UE_LOG(LogDBACore, Verbose, TEXT("[DBAFrontendFlowSubsystem] 忽略已过期的角色选择回调。"));
			return;
		}

		if (SelectedId.IsValid())
		{
			RequestVillageAllocation();
			return;
		}

		BroadcastErrorAndSetState(TEXT("角色选择失败。"), EDBALoginFlowState::CharacterSelecting);
	}));
}

void UDBAFrontendFlowSubsystem::SubmitCharacterCreation(const FDBACharacterCreateRequest& Request)
{
	if (FlowState != EDBALoginFlowState::CharacterCreating)
	{
		BroadcastErrorAndSetState(TEXT("当前状态不允许创建角色。"), EDBALoginFlowState::AwaitingLogin);
		return;
	}
	const uint64 RequestGeneration = BeginFlowRequest();

	UDBAOnlineAccountService* AccountService = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAOnlineAccountService>() : nullptr;
	if (!AccountService)
	{
		BroadcastErrorAndSetState(TEXT("账号服务不可用。"), EDBALoginFlowState::CharacterCreating);
		return;
	}

	AccountService->CreateCharacter(Request, FDBAOnCharacterCreated::CreateWeakLambda(this, [this, RequestGeneration](const FDBACharacterCreateResponse& Response)
	{
		if (!IsFlowRequestCurrent(RequestGeneration))
		{
			UE_LOG(LogDBACore, Verbose, TEXT("[DBAFrontendFlowSubsystem] 忽略已过期的角色创建回调。"));
			return;
		}

		if (Response.bSuccess)
		{
			CachedCharacters.RemoveAll([&Response](const FDBACharacterSummary& Character)
			{
				return Character.CharacterId == Response.CharacterSummary.CharacterId;
			});
			CachedCharacters.Add(Response.CharacterSummary);
			OnCharactersLoaded.Broadcast(CachedCharacters);
			SubmitCharacterSelection(Response.CharacterSummary.CharacterId);
			return;
		}

		BroadcastErrorAndSetState(Response.ErrorMessage, EDBALoginFlowState::CharacterCreating);
	}));
}

void UDBAFrontendFlowSubsystem::EnterCharacterCreate()
{
	if (FlowState == EDBALoginFlowState::CharacterSelecting || FlowState == EDBALoginFlowState::CharacterCreating)
	{
		InvalidatePendingFlowRequests();
		SetFlowState(EDBALoginFlowState::CharacterCreating);
	}
}

void UDBAFrontendFlowSubsystem::BackToCharacterSelect()
{
	if (FlowState == EDBALoginFlowState::CharacterCreating || FlowState == EDBALoginFlowState::CharacterSelecting)
	{
		if (CachedCharacters.IsEmpty())
		{
			BroadcastErrorAndSetState(TEXT("当前没有可选择的角色，请先完成角色创建。"), EDBALoginFlowState::CharacterCreating);
			return;
		}
		InvalidatePendingFlowRequests();
		SetFlowState(EDBALoginFlowState::CharacterSelecting);
	}
}

void UDBAFrontendFlowSubsystem::RefreshCharacterList()
{
	LoadCharactersAfterLogin();
}

void UDBAFrontendFlowSubsystem::RequestVillageAllocation()
{
	UDBAOnlineAccountService* AccountService = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UDBAOnlineAccountService>()
		: nullptr;
	UDBABackendFacadeSubsystem* BackendFacade = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UDBABackendFacadeSubsystem>()
		: nullptr;
	const FDBACharacterId SelectedCharacterId = AccountService ? AccountService->GetCurrentCharacterId() : FDBACharacterId();
	if (!SelectedCharacterId.IsValid() || !BackendFacade)
	{
		BroadcastErrorAndSetState(TEXT("无法申请大厅：角色选择或后端会话服务无效。"), ResolveCharacterFlowState());
		return;
	}

	PendingVillageRequestGeneration = BeginFlowRequest();
	PendingVillageSessionId.Reset();
	VillageConnectionAttempt = 0;
	SetFlowState(EDBALoginFlowState::AllocatingVillage);

	FDBA_GameBackendResponseDelegate Callback;
	Callback.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UDBAFrontendFlowSubsystem, HandleVillageAllocationResponse));
	BackendFacade->AllocateVillage(SelectedCharacterId.ToString(), Callback);
	UE_LOG(LogDBACore, Log, TEXT("[DBAFrontendFlowSubsystem] 已请求后台分配新手村服务器。"));
}

void UDBAFrontendFlowSubsystem::HandleVillageAllocationResponse(
	bool bSuccess,
	const FString& ErrorMessage,
	const FString& DataJson)
{
	if (!IsFlowRequestCurrent(PendingVillageRequestGeneration))
	{
		UE_LOG(LogDBACore, Verbose, TEXT("[DBAFrontendFlowSubsystem] 忽略已过期的新手村分配回调。"));
		return;
	}
	if (!bSuccess || !ParseVillageAllocation(DataJson, PendingVillageSessionId))
	{
		BroadcastErrorAndSetState(
			ErrorMessage.IsEmpty() ? TEXT("大厅服务器分配失败。") : ErrorMessage,
			ResolveCharacterFlowState());
		return;
	}

	SetFlowState(EDBALoginFlowState::WaitingVillageServer);
	RequestVillageConnection();
}

void UDBAFrontendFlowSubsystem::RequestVillageConnection()
{
	if (!IsFlowRequestCurrent(PendingVillageRequestGeneration) || PendingVillageSessionId.IsEmpty())
	{
		return;
	}

	UDBABackendFacadeSubsystem* BackendFacade = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UDBABackendFacadeSubsystem>()
		: nullptr;
	if (!BackendFacade)
	{
		BroadcastErrorAndSetState(TEXT("后端会话服务不可用。"), ResolveCharacterFlowState());
		return;
	}

	++VillageConnectionAttempt;
	FDBA_GameBackendResponseDelegate Callback;
	Callback.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UDBAFrontendFlowSubsystem, HandleVillageConnectionResponse));
	BackendFacade->GetVillageConnection(PendingVillageSessionId, Callback);
}

void UDBAFrontendFlowSubsystem::HandleVillageConnectionResponse(
	bool bSuccess,
	const FString& ErrorMessage,
	const FString& DataJson)
{
	if (!IsFlowRequestCurrent(PendingVillageRequestGeneration))
	{
		return;
	}
	if (!bSuccess)
	{
		const UDBA_GameBackendClientSettings* Settings = GetDefault<UDBA_GameBackendClientSettings>();
		const int32 MaxAttempts = Settings ? FMath::Max(1, Settings->VillageConnectionMaxAttempts) : 1;
		if (VillageConnectionAttempt < MaxAttempts)
		{
			ScheduleVillageConnectionRetry();
			return;
		}

		BroadcastErrorAndSetState(
			ErrorMessage.IsEmpty() ? TEXT("大厅服务器在等待时间内未就绪。") : ErrorMessage,
			ResolveCharacterFlowState());
		return;
	}

	FString TravelUrl;
	if (!UDBA_GameBackendSessionService::TryBuildTravelUrlFromConnectionData(DataJson, PendingVillageSessionId, TravelUrl))
	{
		BroadcastErrorAndSetState(TEXT("新手村连接数据无效。"), ResolveCharacterFlowState());
		return;
	}

	UDBATravelSubsystem* TravelSubsystem = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UDBATravelSubsystem>()
		: nullptr;
	FString TravelError;
	SetFlowState(EDBALoginFlowState::ConnectingVillage);
	if (!TravelSubsystem || !TravelSubsystem->RequestClientTravel(TravelUrl, TravelError))
	{
		BroadcastErrorAndSetState(
			TravelError.IsEmpty() ? TEXT("旅行子系统不可用，无法连接新手村。") : TravelError,
			ResolveCharacterFlowState());
		return;
	}

	UE_LOG(LogDBACore, Log, TEXT("[DBAFrontendFlowSubsystem] 已发起新手村连接，等待 PlayerState 初始化完成。"));
}

void UDBAFrontendFlowSubsystem::ScheduleVillageConnectionRetry()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		BroadcastErrorAndSetState(TEXT("无法等待大厅服务器：当前世界无效。"), ResolveCharacterFlowState());
		return;
	}

	const UDBA_GameBackendClientSettings* Settings = GetDefault<UDBA_GameBackendClientSettings>();
	const float RetryDelay = Settings ? FMath::Max(0.1f, Settings->VillageConnectionRetryDelaySeconds) : 1.0f;
	World->GetTimerManager().SetTimer(
		VillageConnectionRetryTimerHandle,
		this,
		&UDBAFrontendFlowSubsystem::RequestVillageConnection,
		RetryDelay,
		false);
}

void UDBAFrontendFlowSubsystem::ConfirmVillageConnectionReady()
{
	if (FlowState != EDBALoginFlowState::ConnectingVillage)
	{
		return;
	}

	InvalidatePendingFlowRequests();
	SetFlowState(EDBALoginFlowState::InVillage);

	if (UDBABackendFacadeSubsystem* BackendFacade = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UDBABackendFacadeSubsystem>()
		: nullptr)
	{
		BackendFacade->TrackEvent(TEXT("enter_village"));
	}
	UE_LOG(LogDBACore, Log, TEXT("[DBAFrontendFlowSubsystem] 新手村玩家状态已同步，前端流程完成。"));
}

EDBALoginFlowState UDBAFrontendFlowSubsystem::ResolveCharacterFlowState() const
{
	return CachedCharacters.IsEmpty()
		? EDBALoginFlowState::CharacterCreating
		: EDBALoginFlowState::CharacterSelecting;
}
