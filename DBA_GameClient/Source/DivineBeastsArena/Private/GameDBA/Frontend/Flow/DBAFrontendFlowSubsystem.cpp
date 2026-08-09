// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Frontend/Flow/DBAFrontendFlowSubsystem.h"

#include "GameDBA/Frontend/Character/DBACharacterRosterSubsystem.h"
#include "GameDBA/Frontend/Character/DBACharacterCreateDraftSubsystem.h"
#include "GameDBA/Frontend/Core/DBAFrontendErrorMapper.h"

#include "GameDBA/Frontend/Account/DBAOnlineAccountService.h"
#include "GameDBA/Frontend/Online/DBAApiClientSubsystem.h"
#include "GameDBA/Frontend/Preview/DBACharacterPreviewSubsystem.h"
#include "GameDBA/Frontend/Settings/DBAFrontendSettings.h"
#include "GameCore/Core/DBALogChannels.h"
#include "GameBackendClientSettings.h"
#include "GameBackendSessionService.h"
#include "GameDBA/Frontend/Backend/DBABackendFacadeSubsystem.h"
#include "GameDBA/Frontend/ServerDirectory/DBAServerDirectorySubsystem.h"
#include "Dom/JsonObject.h"
#include "Engine/World.h"
#include "GameCore/Networking/Travel/DBATravelSubsystem.h"
#include "Misc/CoreDelegates.h"
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

void UDBAFrontendFlowSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	// Flow 只订阅 ApiClient 的认证结果，不反向让 ApiClient 依赖 UI/Flow 模块。
	Collection.InitializeDependency<UDBAApiClientSubsystem>();
	Super::Initialize(Collection);

	if (UDBAApiClientSubsystem* ApiClient = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAApiClientSubsystem>() : nullptr)
	{
		AuthenticationRefreshFailedHandle = ApiClient->OnAuthenticationRefreshFailed().AddUObject(this, &UDBAFrontendFlowSubsystem::HandleAuthenticationRefreshFailed);
	}

	// 平台生命周期只改变 Flow 会话代次；绝不在此创建 Widget 或 UI Root。
	ApplicationWillEnterBackgroundHandle = FCoreDelegates::ApplicationWillEnterBackgroundDelegate.AddUObject(this, &UDBAFrontendFlowSubsystem::HandleApplicationEnteredBackground);
	ApplicationHasEnteredForegroundHandle = FCoreDelegates::ApplicationHasEnteredForegroundDelegate.AddUObject(this, &UDBAFrontendFlowSubsystem::HandleApplicationResumed);
}

void UDBAFrontendFlowSubsystem::Deinitialize()
{
	if (UDBAApiClientSubsystem* ApiClient = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAApiClientSubsystem>() : nullptr)
	{
		if (AuthenticationRefreshFailedHandle.IsValid())
		{
			ApiClient->OnAuthenticationRefreshFailed().Remove(AuthenticationRefreshFailedHandle);
		}
	}
	if (ApplicationWillEnterBackgroundHandle.IsValid())
	{
		FCoreDelegates::ApplicationWillEnterBackgroundDelegate.Remove(ApplicationWillEnterBackgroundHandle);
	}
	if (ApplicationHasEnteredForegroundHandle.IsValid())
	{
		FCoreDelegates::ApplicationHasEnteredForegroundDelegate.Remove(ApplicationHasEnteredForegroundHandle);
	}
	AuthenticationRefreshFailedHandle.Reset();
	ApplicationWillEnterBackgroundHandle.Reset();
	ApplicationHasEnteredForegroundHandle.Reset();
	Super::Deinitialize();
}

bool UDBAFrontendFlowSubsystem::ShouldEnterCharacterCreate(int32 CharacterCount)
{
	return CharacterCount <= 0;
}

void UDBAFrontendFlowSubsystem::ApplyCharacterRosterSnapshot(const TArray<FDBACharacterSummary>& Characters)
{
	CachedCharacters = Characters;
	OnCharactersLoaded.Broadcast(CachedCharacters);
}

void UDBAFrontendFlowSubsystem::SetSelectedCharacterFromRoster(const FDBACharacterSummary& Character)
{
	FrontendSessionContext.SelectedCharacterId = Character.CharacterId.ToString();
	FrontendSessionContext.Zodiac = Character.Zodiac;
	FrontendSessionContext.Element = Character.PrimaryElement;
	FrontendSessionContext.FiveCamp = Character.FiveCamp;
}

void UDBAFrontendFlowSubsystem::ClearSelectedCharacterFromRoster()
{
	FrontendSessionContext.SelectedCharacterId.Reset();
}

void UDBAFrontendFlowSubsystem::ClearFrontendCharacterContext(const bool bClearServerId)
{
	// 先失效 Flow 回调，再取消/清空各领域子系统，保证换服、登出和维护期间的旧响应无法回写新会话。
	InvalidatePendingFlowRequests();
	if (UDBACharacterRosterSubsystem* Roster = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBACharacterRosterSubsystem>() : nullptr)
	{
		Roster->ClearCache();
	}
	if (UDBACharacterPreviewSubsystem* Preview = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBACharacterPreviewSubsystem>() : nullptr)
	{
		Preview->ReleasePreview();
	}
	if (UDBACharacterCreateDraftSubsystem* Draft = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBACharacterCreateDraftSubsystem>() : nullptr)
	{
		Draft->ResetDraft();
	}

	CachedCharacters.Reset();
	FrontendSessionContext.ResetCharacterDraft();
	if (bClearServerId)
	{
		FrontendSessionContext.ServerId.Reset();
	}
}

void UDBAFrontendFlowSubsystem::RefreshServerDirectory()
{
	const UDBAExternalServiceSettings* ServiceSettings = GetDefault<UDBAExternalServiceSettings>();
	if (UDBAServerDirectorySubsystem* ServerDirectory = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAServerDirectorySubsystem>() : nullptr)
	{
		ServerDirectory->RefreshDirectory(
			ServiceSettings ? ServiceSettings->Region : FString(),
			ServiceSettings ? ServiceSettings->ApiVersion : FString(),
			ServiceSettings ? ServiceSettings->Platform : FString());
	}
}

void UDBAFrontendFlowSubsystem::HandleAuthenticationRefreshFailed()
{
	// ApiClient 已完成单飞 Refresh 且失败；统一从这里登出，避免角色页、选服页各自清理会话。
	UE_LOG(LogDBAOnline, Warning, TEXT("前台认证刷新失败，开始统一登出与状态清理。"));
	RequestLogout();
}

void UDBAFrontendFlowSubsystem::HandleApplicationEnteredBackground()
{
	bApplicationWasSuspended = true;
	// 仅失效当前异步代次，不创建/销毁 UI；恢复时会按当时 Screen 检查会话并按需刷新。
	InvalidatePendingFlowRequests();
}

void UDBAFrontendFlowSubsystem::HandleApplicationResumed()
{
	if (!bApplicationWasSuspended || bResumeSessionRefreshInFlight)
	{
		return;
	}
	bApplicationWasSuspended = false;

	const EDBAFrontendState ResumeState = GetFrontendState();
	if (ResumeState == EDBAFrontendState::Login || ResumeState == EDBAFrontendState::Register || FrontendSessionContext.AccountId.IsEmpty())
	{
		return;
	}

	UDBAOnlineAccountService* AccountService = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAOnlineAccountService>() : nullptr;
	if (!AccountService)
	{
		HandleTokenExpired();
		return;
	}

	bResumeSessionRefreshInFlight = true;
	const uint64 RequestGeneration = BeginFlowRequest();
	AccountService->RefreshSession(FDBAOnLoginComplete::CreateWeakLambda(this, [this, ResumeState, RequestGeneration](const FDBALoginResponse& Response)
	{
		if (!IsFlowRequestCurrent(RequestGeneration))
		{
			return;
		}
		bResumeSessionRefreshInFlight = false;
		if (!Response.bSuccess)
		{
			HandleTokenExpired();
			return;
		}

		// Refresh 成功只确认会话有效，不重新创建 UI Root，也不重置仍在编辑的角色 Draft。
		FrontendSessionContext.AccountId = Response.AccountInfo.AccountId.ToString();
		if (ResumeState == EDBAFrontendState::ServerSelect)
		{
			RefreshServerDirectory();
		}
		else if (ResumeState == EDBAFrontendState::CharacterSelect || ResumeState == EDBAFrontendState::CharacterRosterLoading)
		{
			RefreshCharacterList();
		}
	}));
}

void UDBAFrontendFlowSubsystem::SetFlowState(EDBALoginFlowState NewState)
{
	const EDBAFrontendState TargetState = ResolveFrontendStateForLegacyState(NewState);
	if (!TryTransitionTo(TargetState))
	{
		UE_LOG(LogDBAFrontend, Warning, TEXT("[DBAFrontendFlowSubsystem] 拒绝旧状态到前台状态的非法转换：旧状态=%d，目标状态=%d。"), static_cast<int32>(NewState), static_cast<int32>(TargetState));
		return;
	}
	if (FlowState != NewState)
	{
		FlowState = NewState;
		OnFlowStateChanged.Broadcast(NewState);
	}
}

bool UDBAFrontendFlowSubsystem::TryTransitionTo(const EDBAFrontendState NewState)
{
	const EDBAFrontendState PreviousState = FrontendSessionContext.ClientSessionState;
	if (!DBAFrontendStateMachine::CanTransition(PreviousState, NewState))
	{
		UE_LOG(LogDBAFrontend, Warning, TEXT("[DBAFrontendFlowSubsystem] 拒绝非法前台状态转换：%d -> %d。"), static_cast<int32>(PreviousState), static_cast<int32>(NewState));
		return false;
	}

	if (PreviousState == NewState)
	{
		return true;
	}

	FrontendSessionContext.ClientSessionState = NewState;
	UE_LOG(LogDBAFrontend, Log, TEXT("[DBAFrontendFlowSubsystem] 前台状态切换：%d -> %d。"), static_cast<int32>(PreviousState), static_cast<int32>(NewState));
	OnFrontendStateChanged.Broadcast(PreviousState, NewState);
	return true;
}

void UDBAFrontendFlowSubsystem::UpdateLegacyFlowState(const EDBAFrontendState State)
{
	EDBALoginFlowState NewLegacyState = EDBALoginFlowState::AwaitingLogin;
	switch (State)
	{
	case EDBAFrontendState::Bootstrapping:
	case EDBAFrontendState::Startup:
		NewLegacyState = EDBALoginFlowState::Booting;
		break;
	case EDBAFrontendState::CharacterRosterLoading:
		NewLegacyState = EDBALoginFlowState::LoadingCharacters;
		break;
	case EDBAFrontendState::ServerSelect:
		NewLegacyState = EDBALoginFlowState::ServerSelecting;
		break;
	case EDBAFrontendState::CharacterSelect:
		NewLegacyState = EDBALoginFlowState::CharacterSelecting;
		break;
	case EDBAFrontendState::CharacterCreate_Zodiac:
	case EDBAFrontendState::CharacterCreate_Element:
	case EDBAFrontendState::CharacterCreate_FiveCamp:
	case EDBAFrontendState::CharacterCreate_Confirm:
		NewLegacyState = EDBALoginFlowState::CharacterCreating;
		break;
	case EDBAFrontendState::EnteringWorld:
		NewLegacyState = EDBALoginFlowState::ConnectingVillage;
		break;
	case EDBAFrontendState::RecoverableError:
		NewLegacyState = EDBALoginFlowState::RecoverableError;
		break;
	case EDBAFrontendState::FatalError:
		NewLegacyState = EDBALoginFlowState::FatalError;
		break;
	default:
		NewLegacyState = EDBALoginFlowState::AwaitingLogin;
		break;
	}

	if (FlowState != NewLegacyState)
	{
		FlowState = NewLegacyState;
		OnFlowStateChanged.Broadcast(NewLegacyState);
	}
}

EDBAFrontendState UDBAFrontendFlowSubsystem::ResolveFrontendStateForLegacyState(const EDBALoginFlowState LegacyState) const
{
	switch (LegacyState)
	{
	case EDBALoginFlowState::Booting: return EDBAFrontendState::Bootstrapping;
	case EDBALoginFlowState::AwaitingLogin:
	case EDBALoginFlowState::Authenticating: return EDBAFrontendState::Login;
	case EDBALoginFlowState::ServerSelecting: return EDBAFrontendState::ServerSelect;
	case EDBALoginFlowState::LoadingCharacters: return EDBAFrontendState::CharacterRosterLoading;
	case EDBALoginFlowState::CharacterSelecting: return EDBAFrontendState::CharacterSelect;
	case EDBALoginFlowState::CharacterCreating:
		return DBAFrontendStateMachine::IsCharacterCreationState(FrontendSessionContext.ClientSessionState)
			? FrontendSessionContext.ClientSessionState
			: EDBAFrontendState::CharacterCreate_Zodiac;
	case EDBALoginFlowState::AllocatingVillage:
	case EDBALoginFlowState::WaitingVillageServer:
	case EDBALoginFlowState::ConnectingVillage:
	case EDBALoginFlowState::InitializingVillage:
	case EDBALoginFlowState::InVillage: return EDBAFrontendState::EnteringWorld;
	case EDBALoginFlowState::RecoverableError: return EDBAFrontendState::RecoverableError;
	case EDBALoginFlowState::FatalError: return EDBAFrontendState::FatalError;
	default: return EDBAFrontendState::Login;
	}
}

uint64 UDBAFrontendFlowSubsystem::BeginFlowRequest()
{
	return ++FlowRequestGeneration;
}

void UDBAFrontendFlowSubsystem::InvalidatePendingFlowRequests()
{
	++FlowRequestGeneration;
	bAuthenticationRequestInFlight = false;
}

bool UDBAFrontendFlowSubsystem::IsFlowRequestCurrent(uint64 RequestGeneration) const
{
	return RequestGeneration == FlowRequestGeneration;
}

void UDBAFrontendFlowSubsystem::BroadcastErrorAndSetState(const FString& ErrorMessage, EDBALoginFlowState NewState)
{
	OnFlowApiError.Broadcast(UDBAFrontendErrorMapper::FromLegacyMessage(ErrorMessage));
	OnFlowError.Broadcast(ErrorMessage);
	ForceRecoverableErrorAndFallback(ResolveFrontendStateForLegacyState(NewState), ErrorMessage);
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
	FrontendSessionContext = FDBAFrontendSessionContext();
	bAuthenticationRequestInFlight = false;
	FrontendSessionContext.ClientSessionState = EDBAFrontendState::Bootstrapping;
	UpdateLegacyFlowState(EDBAFrontendState::Bootstrapping);
	TryTransitionTo(EDBAFrontendState::Startup);

	const UDBAFrontendSettings* Settings = GetDefault<UDBAFrontendSettings>();
	if (Settings && Settings->bAttemptAutoLogin)
	{
		StartAutoLogin();
		return;
	}

	TryTransitionTo(EDBAFrontendState::Login);
	UpdateLegacyFlowState(EDBAFrontendState::Login);
}

void UDBAFrontendFlowSubsystem::StartAutoLogin()
{
	if (bAuthenticationRequestInFlight)
	{
		UE_LOG(LogDBAOnline, Verbose, TEXT("[DBAFrontendFlowSubsystem] 已有认证请求进行中，忽略重复自动登录请求。"));
		return;
	}
	if (!TryTransitionTo(EDBAFrontendState::AutoLogin))
	{
		return;
	}
	UpdateLegacyFlowState(EDBAFrontendState::AutoLogin);
	bAuthenticationRequestInFlight = true;
	const uint64 RequestGeneration = BeginFlowRequest();
	UDBAOnlineAccountService* AccountService = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAOnlineAccountService>() : nullptr;
	if (!AccountService)
	{
		bAuthenticationRequestInFlight = false;
		TryTransitionTo(EDBAFrontendState::Login);
		UpdateLegacyFlowState(EDBAFrontendState::Login);
		return;
	}

	AccountService->TryAutoLogin(FDBAOnLoginComplete::CreateWeakLambda(this, [this, RequestGeneration](const FDBALoginResponse& Response)
	{
		if (!IsFlowRequestCurrent(RequestGeneration))
		{
			return;
		}
		bAuthenticationRequestInFlight = false;
		if (!Response.bSuccess)
		{
			// 自动登录失败是正常回退，既不向 Widget 传递远端原文，也不阻塞手动登录。
			TryTransitionTo(EDBAFrontendState::Login);
			UpdateLegacyFlowState(EDBAFrontendState::Login);
			return;
		}
		HandleLoginSucceeded(Response);
	}));
}

void UDBAFrontendFlowSubsystem::HandleLoginSucceeded(const FDBALoginResponse& Response)
{
	FrontendSessionContext.AccountId = Response.AccountInfo.AccountId.ToString();
	if (TryTransitionTo(EDBAFrontendState::ServerSelect))
	{
		UpdateLegacyFlowState(EDBAFrontendState::ServerSelect);
		const UDBAExternalServiceSettings* ServiceSettings = GetDefault<UDBAExternalServiceSettings>();
		if (UDBAServerDirectorySubsystem* ServerDirectory = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAServerDirectorySubsystem>() : nullptr)
		{
			ServerDirectory->RefreshDirectory(
				ServiceSettings ? ServiceSettings->Region : FString(),
				ServiceSettings ? ServiceSettings->ClientVersion : FString(),
				ServiceSettings ? ServiceSettings->Platform : FString());
		}
	}
}

void UDBAFrontendFlowSubsystem::ForceRecoverableErrorAndFallback(const EDBAFrontendState FallbackState, const FString& ErrorMessage)
{
	const EDBAFrontendState CurrentState = GetFrontendState();
	if (CurrentState != EDBAFrontendState::RecoverableError)
	{
		TryTransitionTo(EDBAFrontendState::RecoverableError);
		UpdateLegacyFlowState(EDBAFrontendState::RecoverableError);
	}

	if (!TryTransitionTo(FallbackState))
	{
		TryTransitionTo(EDBAFrontendState::Login);
		UpdateLegacyFlowState(EDBAFrontendState::Login);
	}
	else
	{
		UpdateLegacyFlowState(FallbackState);
	}

	UE_LOG(LogDBAFrontend, Warning, TEXT("[DBAFrontendFlowSubsystem] 前台流程已从可恢复错误回退：%s"), *ErrorMessage);
}

void UDBAFrontendFlowSubsystem::BeginRegistration()
{
	if (TryTransitionTo(EDBAFrontendState::Register))
	{
		UpdateLegacyFlowState(EDBAFrontendState::Register);
	}
}

void UDBAFrontendFlowSubsystem::SubmitRegistration(const FString& Account, const FString& Password)
{
	if (GetFrontendState() != EDBAFrontendState::Register)
	{
		UE_LOG(LogDBAOnline, Warning, TEXT("[DBAFrontendFlowSubsystem] 当前前台状态不允许提交账号注册。"));
		return;
	}
	if (bAuthenticationRequestInFlight)
	{
		UE_LOG(LogDBAOnline, Verbose, TEXT("[DBAFrontendFlowSubsystem] 已有认证请求进行中，忽略重复注册请求。"));
		return;
	}

	const FString NormalizedAccount = Account.TrimStartAndEnd();
	if (NormalizedAccount.IsEmpty() || Password.IsEmpty())
	{
		OnFlowApiError.Broadcast(UDBAFrontendErrorMapper::FromLegacyMessage(TEXT("账号和密码不能为空。")));
		OnFlowError.Broadcast(TEXT("账号和密码不能为空。"));
		return;
	}

	bAuthenticationRequestInFlight = true;
	const uint64 RequestGeneration = BeginFlowRequest();
	FlowState = EDBALoginFlowState::Authenticating;
	OnFlowStateChanged.Broadcast(FlowState);

	UDBAOnlineAccountService* AccountService = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAOnlineAccountService>() : nullptr;
	if (!AccountService)
	{
		bAuthenticationRequestInFlight = false;
		ForceRecoverableErrorAndFallback(EDBAFrontendState::Register, TEXT("账号服务不可用。"));
		return;
	}

	AccountService->RegisterAccount(NormalizedAccount, Password, FDBAOnLoginComplete::CreateWeakLambda(this, [this, RequestGeneration](const FDBALoginResponse& Response)
	{
		if (!IsFlowRequestCurrent(RequestGeneration))
		{
			UE_LOG(LogDBAOnline, Verbose, TEXT("[DBAFrontendFlowSubsystem] 忽略已过期的账号注册回调。"));
			return;
		}

		bAuthenticationRequestInFlight = false;
		if (Response.bSuccess)
		{
			HandleLoginSucceeded(Response);
			return;
		}

		OnFlowApiError.Broadcast(UDBAFrontendErrorMapper::FromLegacyMessage(Response.ErrorMessage));
		OnFlowError.Broadcast(Response.ErrorMessage);
		ForceRecoverableErrorAndFallback(EDBAFrontendState::Register, Response.ErrorMessage);
	}));
}

void UDBAFrontendFlowSubsystem::CancelRegistration()
{
	if (TryTransitionTo(EDBAFrontendState::Login))
	{
		UpdateLegacyFlowState(EDBAFrontendState::Login);
	}
}

void UDBAFrontendFlowSubsystem::BeginServerSelection()
{
	if (TryTransitionTo(EDBAFrontendState::ServerSelect))
	{
		UpdateLegacyFlowState(EDBAFrontendState::ServerSelect);
	}
}

void UDBAFrontendFlowSubsystem::SelectServer(const FString& ServerId)
{
	if (GetFrontendState() != EDBAFrontendState::ServerSelect || ServerId.TrimStartAndEnd().IsEmpty())
	{
		UE_LOG(LogDBAOnline, Warning, TEXT("[DBAFrontendFlowSubsystem] 拒绝无效选服请求。"));
		return;
	}
	UDBAServerDirectorySubsystem* ServerDirectory = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAServerDirectorySubsystem>() : nullptr;
	if (!ServerDirectory || !ServerDirectory->FindSelectableServer(ServerId.TrimStartAndEnd()))
	{
		UE_LOG(LogDBAOnline, Warning, TEXT("[DBAFrontendFlowSubsystem] 拒绝选择不存在、不可用或尚未加载完成的区服。"));
		OnFlowApiError.Broadcast(UDBAFrontendErrorMapper::FromLegacyMessage(TEXT("所选区服当前不可进入，请刷新区服列表后重试。")));
		OnFlowError.Broadcast(TEXT("所选区服当前不可进入，请刷新区服列表后重试。"));
		return;
	}

	if (UDBACharacterRosterSubsystem* Roster = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBACharacterRosterSubsystem>() : nullptr)
	{
		Roster->ClearCache();
	}
	FrontendSessionContext.ServerId = ServerId.TrimStartAndEnd();
	ServerDirectory->RecordLastSelectedServer(FrontendSessionContext.AccountId, FrontendSessionContext.ServerId);
	LoadCharactersAfterLogin();
}

void UDBAFrontendFlowSubsystem::SubmitLogin(const FString& Email, const FString& Password)
{
	if (GetFrontendState() != EDBAFrontendState::Login)
	{
		UE_LOG(LogDBAOnline, Warning, TEXT("[DBAFrontendFlowSubsystem] 当前前台状态不允许提交账号登录。"));
		return;
	}
	if (bAuthenticationRequestInFlight)
	{
		UE_LOG(LogDBAOnline, Verbose, TEXT("[DBAFrontendFlowSubsystem] 已有认证请求进行中，忽略重复账号登录请求。"));
		return;
	}
	bAuthenticationRequestInFlight = true;
	const uint64 RequestGeneration = BeginFlowRequest();
	FlowState = EDBALoginFlowState::Authenticating;
	OnFlowStateChanged.Broadcast(FlowState);

	UDBAOnlineAccountService* AccountService = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAOnlineAccountService>() : nullptr;
	if (!AccountService)
	{
		bAuthenticationRequestInFlight = false;
		BroadcastErrorAndSetState(TEXT("账号服务不可用。"), EDBALoginFlowState::AwaitingLogin);
		return;
	}

	AccountService->LoginWithCredentials(Email, Password, FDBAOnLoginComplete::CreateWeakLambda(this, [this, RequestGeneration](const FDBALoginResponse& Response)
	{
		if (!IsFlowRequestCurrent(RequestGeneration))
		{
			UE_LOG(LogDBAOnline, Verbose, TEXT("[DBAFrontendFlowSubsystem] 忽略已过期的账号登录回调。"));
			return;
		}
		bAuthenticationRequestInFlight = false;

		if (Response.bSuccess)
		{
			HandleLoginSucceeded(Response);
			return;
		}

		BroadcastErrorAndSetState(Response.ErrorMessage, EDBALoginFlowState::AwaitingLogin);
	}));
}

void UDBAFrontendFlowSubsystem::SubmitGuestLogin()
{
	if (GetFrontendState() != EDBAFrontendState::Login)
	{
		UE_LOG(LogDBAOnline, Warning, TEXT("[DBAFrontendFlowSubsystem] 当前前台状态不允许提交游客登录。"));
		return;
	}
	const UDBAFrontendSettings* Settings = GetDefault<UDBAFrontendSettings>();
	if (Settings && !Settings->bEnableGuestLogin)
	{
		UE_LOG(LogDBAOnline, Warning, TEXT("[DBAFrontendFlowSubsystem] 当前配置已关闭游客登录入口。"));
		OnFlowApiError.Broadcast(UDBAFrontendErrorMapper::FromLegacyMessage(TEXT("当前版本未开放游客登录。")));
		OnFlowError.Broadcast(TEXT("当前版本未开放游客登录。"));
		return;
	}
	if (bAuthenticationRequestInFlight)
	{
		UE_LOG(LogDBAOnline, Verbose, TEXT("[DBAFrontendFlowSubsystem] 已有认证请求进行中，忽略重复游客登录请求。"));
		return;
	}
	bAuthenticationRequestInFlight = true;
	const uint64 RequestGeneration = BeginFlowRequest();
	FlowState = EDBALoginFlowState::Authenticating;
	OnFlowStateChanged.Broadcast(FlowState);

	UDBAOnlineAccountService* AccountService = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAOnlineAccountService>() : nullptr;
	if (!AccountService)
	{
		bAuthenticationRequestInFlight = false;
		BroadcastErrorAndSetState(TEXT("账号服务不可用。"), EDBALoginFlowState::AwaitingLogin);
		return;
	}

	AccountService->GuestLogin(FDBAOnLoginComplete::CreateWeakLambda(this, [this, RequestGeneration](const FDBALoginResponse& Response)
	{
		if (!IsFlowRequestCurrent(RequestGeneration))
		{
			UE_LOG(LogDBAOnline, Verbose, TEXT("[DBAFrontendFlowSubsystem] 忽略已过期的游客登录回调。"));
			return;
		}
		bAuthenticationRequestInFlight = false;

		if (Response.bSuccess)
		{
			HandleLoginSucceeded(Response);
			return;
		}

		BroadcastErrorAndSetState(Response.ErrorMessage, EDBALoginFlowState::AwaitingLogin);
	}));
}

void UDBAFrontendFlowSubsystem::LoadCharactersAfterLogin()
{
	const EDBAFrontendState PreviousFrontendState = GetFrontendState();
	if (!SynchronizeBackendAuthentication())
	{
		ForceRecoverableErrorAndFallback(PreviousFrontendState, TEXT("后端认证上下文同步失败。"));
		return;
	}

	if (!TryTransitionTo(EDBAFrontendState::CharacterRosterLoading))
	{
		ForceRecoverableErrorAndFallback(PreviousFrontendState, TEXT("当前状态无法加载角色列表。"));
		return;
	}
	UpdateLegacyFlowState(EDBAFrontendState::CharacterRosterLoading);
	const uint64 RequestGeneration = BeginFlowRequest();
	UDBACharacterRosterSubsystem* Roster = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBACharacterRosterSubsystem>() : nullptr;
	if (!Roster)
	{
		ForceRecoverableErrorAndFallback(PreviousFrontendState, TEXT("账号服务不可用。"));
		return;
	}

	Roster->RefreshCharacterList(FrontendSessionContext.ServerId, [this, PreviousFrontendState, RequestGeneration, Roster](const FDBAOperationResult& Result)
	{
		if (!IsFlowRequestCurrent(RequestGeneration))
		{
			UE_LOG(LogDBACharacter, Verbose, TEXT("[DBAFrontendFlowSubsystem] 忽略已过期的角色列表回调。"));
			return;
		}

		if (!Result.bSuccess)
		{
			UE_LOG(LogDBACharacter, Warning, TEXT("[DBAFrontendFlowSubsystem] 角色列表加载失败，不进入创建流程。"));
			ForceRecoverableErrorAndFallback(PreviousFrontendState, TEXT("获取角色列表失败。"));
			return;
		}

		CachedCharacters = Roster->GetCachedCharacters();
		UE_LOG(LogDBACharacter, Log, TEXT("[DBAFrontendFlowSubsystem] 角色列表加载完成：%d"), CachedCharacters.Num());
		const EDBAFrontendState TargetState = ShouldEnterCharacterCreate(CachedCharacters.Num())
			? EDBAFrontendState::CharacterCreate_Zodiac
			: EDBAFrontendState::CharacterSelect;
		if (TargetState == EDBAFrontendState::CharacterCreate_Zodiac)
		{
			if (UDBACharacterCreateDraftSubsystem* Draft = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBACharacterCreateDraftSubsystem>() : nullptr)
			{
				Draft->BeginDraft();
			}
			FrontendSessionContext.ResetCharacterDraft();
		}
		if (TryTransitionTo(TargetState))
		{
			UpdateLegacyFlowState(TargetState);
		}
	});
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
	if (GetFrontendState() != EDBAFrontendState::CharacterSelect
		&& GetFrontendState() != EDBAFrontendState::CharacterCreate_Confirm)
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

	UDBACharacterRosterSubsystem* Roster = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBACharacterRosterSubsystem>() : nullptr;
	if (!Roster)
	{
		BroadcastErrorAndSetState(TEXT("账号服务不可用。"), EDBALoginFlowState::CharacterSelecting);
		return;
	}

	Roster->SelectCharacter(CharacterId, [this, RequestGeneration](const FDBAOperationResult& Result, const FDBACharacterDetails& Details)
	{
		if (!IsFlowRequestCurrent(RequestGeneration))
		{
			UE_LOG(LogDBACharacter, Verbose, TEXT("[DBAFrontendFlowSubsystem] 忽略已过期的角色选择回调。"));
			return;
		}

		if (Result.bSuccess && Details.Summary.CharacterId.IsValid())
		{
			TryTransitionTo(EDBAFrontendState::EnteringWorld);
			RequestVillageAllocation();
			return;
		}

		BroadcastErrorAndSetState(TEXT("角色选择失败。"), EDBALoginFlowState::CharacterSelecting);
	});
}

void UDBAFrontendFlowSubsystem::SubmitCharacterCreation(const FDBACharacterCreateRequest& Request)
{
	if (bCharacterCreateRequestInFlight)
	{
		// 同一 Confirm 页面只能存在一个在途请求；重复点击不会生成第二个角色或第二个幂等键。
		// 不广播“完成”事件，避免表现层错误地关闭首个请求的 Loading 状态。
		UE_LOG(LogDBACharacter, Verbose, TEXT("[DBAFrontendFlowSubsystem] 已忽略重复的角色创建提交。"));
		return;
	}
	// Flow 只在确认页发送创建意图，禁止任意 Widget 从中间步骤直接创建角色。
	if (GetFrontendState() != EDBAFrontendState::CharacterCreate_Confirm)
	{
		UE_LOG(LogDBACharacter, Warning, TEXT("[DBAFrontendFlowSubsystem] 角色创建请求未通过完整状态机步骤。"));
		OnCharacterCreateCompleted.Broadcast(FDBAOperationResult::Failure(EDBAErrorCode::InvalidState, TEXT("当前不在角色创建确认步骤。")), FDBACharacterSummary());
		return;
	}

	if (FlowState != EDBALoginFlowState::CharacterCreating)
	{
		BroadcastErrorAndSetState(TEXT("当前状态不允许创建角色。"), EDBALoginFlowState::AwaitingLogin);
		OnCharacterCreateCompleted.Broadcast(FDBAOperationResult::Failure(EDBAErrorCode::InvalidState, TEXT("当前流程不允许创建角色。")), FDBACharacterSummary());
		return;
	}
	const uint64 RequestGeneration = BeginFlowRequest();

	UDBACharacterRosterSubsystem* Roster = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBACharacterRosterSubsystem>() : nullptr;
	if (!Roster)
	{
		BroadcastErrorAndSetState(TEXT("账号服务不可用。"), EDBALoginFlowState::CharacterCreating);
		OnCharacterCreateCompleted.Broadcast(FDBAOperationResult::Failure(EDBAErrorCode::ServiceUnavailable, TEXT("角色服务不可用。")), FDBACharacterSummary());
		return;
	}

	UDBACharacterCreateDraftSubsystem* DraftSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBACharacterCreateDraftSubsystem>() : nullptr;
	FDBACharacterCreateRequest DraftRequest;
	FText DraftReason;
	if (!DraftSubsystem || !DraftSubsystem->BuildCreateRequest(DraftRequest, DraftReason))
	{
		BroadcastErrorAndSetState(DraftReason.IsEmpty() ? TEXT("角色创建草稿无效。") : DraftReason.ToString(), EDBALoginFlowState::CharacterCreating);
		OnCharacterCreateCompleted.Broadcast(FDBAOperationResult::Failure(EDBAErrorCode::InvalidData, DraftReason.IsEmpty() ? TEXT("角色创建草稿无效。") : DraftReason.ToString()), FDBACharacterSummary());
		return;
	}

	// 保持旧 Widget 调用兼容；新业务只提交 Draft 构建的请求，避免 Widget 临时变量覆盖已验证外观。
	(void)Request;
	bCharacterCreateRequestInFlight = true;
	if (PendingCharacterCreateIdempotencyKey.IsEmpty())
	{
		PendingCharacterCreateIdempotencyKey = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphensLower);
	}
	// RequestGeneration 已在通过 Flow 状态与 Draft 校验后生成；同一次创建不得再开启第二个代次。
	const FDBACharacterAppearance DraftAppearance = DraftSubsystem->GetDraft().Appearance;
	Roster->CreateCharacter(DraftRequest, DraftAppearance, PendingCharacterCreateIdempotencyKey, [this, RequestGeneration](const FDBAOperationResult& Result, const FDBACharacterDetails& Details)
	{
		if (!IsFlowRequestCurrent(RequestGeneration))
		{
			UE_LOG(LogDBACharacter, Verbose, TEXT("[DBAFrontendFlowSubsystem] 忽略已过期的角色创建回调。"));
			return;
		}

		bCharacterCreateRequestInFlight = false;
		if (Result.bSuccess)
		{
			// 服务端创建成功后立即清理本地草稿，再选中新角色；失败时保留草稿供用户修正重试。
			if (UDBACharacterCreateDraftSubsystem* Draft = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBACharacterCreateDraftSubsystem>() : nullptr)
			{
				Draft->ResetDraft();
			}
			if (UDBACharacterRosterSubsystem* CurrentRoster = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBACharacterRosterSubsystem>() : nullptr)
			{
				// 新角色已由服务端创建；这里只写入前台选中态并回到角色选择，绝不自动 ClientTravel。
				CurrentRoster->SelectCreatedCharacterForFrontend(Details);
			}
			PendingCharacterCreateIdempotencyKey.Reset();
			TryTransitionTo(EDBAFrontendState::CharacterSelect);
			UpdateLegacyFlowState(EDBAFrontendState::CharacterSelect);
			OnCharacterCreateCompleted.Broadcast(Result, Details.Summary);
			return;
		}

		if (!Result.ApiError.bCanRetry)
		{
			PendingCharacterCreateIdempotencyKey.Reset();
		}
		OnCharacterCreateCompleted.Broadcast(Result, FDBACharacterSummary());
		BroadcastErrorAndSetState(Result.ErrorMessage, EDBALoginFlowState::CharacterCreating);
	});
}

void UDBAFrontendFlowSubsystem::CancelCharacterCreationSubmission()
{
	if (!bCharacterCreateRequestInFlight || GetFrontendState() != EDBAFrontendState::CharacterCreate_Confirm)
	{
		return;
	}
	// 先关闭本地门闩并失效回调，再取消 HTTP，避免 CancelRequest 同步回调时刷新已关闭的确认页。
	bCharacterCreateRequestInFlight = false;
	InvalidatePendingFlowRequests();
	if (UDBACharacterRosterSubsystem* Roster = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBACharacterRosterSubsystem>() : nullptr)
	{
		Roster->CancelCreateCharacterRequest();
	}
	// 保留 Draft 与幂等键，避免服务端已接收时重试创建第二个角色。
}

void UDBAFrontendFlowSubsystem::EnterCharacterCreate()
{
	if (GetFrontendState() == EDBAFrontendState::CharacterSelect)
	{
		InvalidatePendingFlowRequests();
		bCharacterCreateRequestInFlight = false;
		PendingCharacterCreateIdempotencyKey.Reset();
		// 显式入口必须重新开始草稿，防止已取消创建的输入被重新使用。
		if (UDBACharacterCreateDraftSubsystem* Draft = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBACharacterCreateDraftSubsystem>() : nullptr)
		{
			Draft->BeginDraft();
		}
		FrontendSessionContext.ResetCharacterDraft();
		if (TryTransitionTo(EDBAFrontendState::CharacterCreate_Zodiac))
		{
			UpdateLegacyFlowState(EDBAFrontendState::CharacterCreate_Zodiac);
		}
	}
}

void UDBAFrontendFlowSubsystem::BackToCharacterSelect()
{
	CancelCharacterCreation();
}

void UDBAFrontendFlowSubsystem::CancelCharacterCreation()
{
	if (DBAFrontendStateMachine::IsCharacterCreationState(GetFrontendState()))
	{
		InvalidatePendingFlowRequests();
		// 取消是清理语义，不保留为服务端角色；临时恢复仅在后续明确保存时才由调用方执行。
		if (UDBACharacterCreateDraftSubsystem* Draft = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBACharacterCreateDraftSubsystem>() : nullptr)
		{
			Draft->ResetDraft();
		}
		FrontendSessionContext.ResetCharacterDraft();
		if (TryTransitionTo(EDBAFrontendState::CharacterSelect))
		{
			UpdateLegacyFlowState(EDBAFrontendState::CharacterSelect);
		}
	}
}

bool UDBAFrontendFlowSubsystem::SelectCharacterCreateZodiac(const EDBAZodiac Zodiac)
{
	UDBACharacterCreateDraftSubsystem* Draft = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBACharacterCreateDraftSubsystem>() : nullptr;
	FText Reason;
	if (GetFrontendState() != EDBAFrontendState::CharacterCreate_Zodiac || !Draft
		|| (Draft->GetDraft().ZodiacType != Zodiac && !Draft->SetZodiac(Zodiac)) || !Draft->Next(Reason))
	{
		return false;
	}
	FrontendSessionContext.Zodiac = Draft->GetDraft().ZodiacType;
	if (!TryTransitionTo(EDBAFrontendState::CharacterCreate_Element))
	{
		return false;
	}
	UpdateLegacyFlowState(EDBAFrontendState::CharacterCreate_Element);
	return true;
}

bool UDBAFrontendFlowSubsystem::SelectCharacterCreateElement(const EDBAElement Element)
{
	UDBACharacterCreateDraftSubsystem* Draft = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBACharacterCreateDraftSubsystem>() : nullptr;
	FText Reason;
	if (GetFrontendState() != EDBAFrontendState::CharacterCreate_Element || !Draft
		|| (Draft->GetDraft().ElementType != Element && !Draft->SetElement(Element)) || !Draft->Next(Reason))
	{
		return false;
	}
	FrontendSessionContext.Element = Draft->GetDraft().ElementType;
	if (!TryTransitionTo(EDBAFrontendState::CharacterCreate_FiveCamp))
	{
		return false;
	}
	UpdateLegacyFlowState(EDBAFrontendState::CharacterCreate_FiveCamp);
	return true;
}

bool UDBAFrontendFlowSubsystem::SelectCharacterCreateFiveCamp(const EDBAFiveCamp FiveCamp)
{
	UDBACharacterCreateDraftSubsystem* Draft = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBACharacterCreateDraftSubsystem>() : nullptr;
	FText Reason;
	if (GetFrontendState() != EDBAFrontendState::CharacterCreate_FiveCamp || !Draft
		|| (Draft->GetDraft().FiveCampType != FiveCamp && !Draft->SetFiveCamp(FiveCamp)) || !Draft->Next(Reason))
	{
		return false;
	}
	FrontendSessionContext.FiveCamp = Draft->GetDraft().FiveCampType;
	if (!TryTransitionTo(EDBAFrontendState::CharacterCreate_Confirm))
	{
		return false;
	}
	UpdateLegacyFlowState(EDBAFrontendState::CharacterCreate_Confirm);
	return true;
}

bool UDBAFrontendFlowSubsystem::AdvanceCharacterCreateDraft()
{
	// 统一从当前 Flow 状态取值推进，保证 Flow 路由与 Draft 的业务步骤不会分叉。
	UDBACharacterCreateDraftSubsystem* Draft = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBACharacterCreateDraftSubsystem>() : nullptr;
	if (!Draft)
	{
		return false;
	}

	switch (GetFrontendState())
	{
	case EDBAFrontendState::CharacterCreate_Zodiac:
		return SelectCharacterCreateZodiac(Draft->GetDraft().ZodiacType);
	case EDBAFrontendState::CharacterCreate_Element:
		return SelectCharacterCreateElement(Draft->GetDraft().ElementType);
	case EDBAFrontendState::CharacterCreate_FiveCamp:
		return SelectCharacterCreateFiveCamp(Draft->GetDraft().FiveCampType);
	case EDBAFrontendState::CharacterCreate_Confirm:
	{
		FText Reason;
		if (!Draft->Validate(Reason))
		{
			UE_LOG(LogDBACharacter, Warning, TEXT("角色创建确认步骤未通过：%s"), *Reason.ToString());
			return false;
		}
		SubmitCharacterCreation(FDBACharacterCreateRequest());
		return true;
	}
	default:
		return false;
	}
}

void UDBAFrontendFlowSubsystem::BackCharacterCreateStep()
{
	// 第一页没有上一步，因此按产品回退策略取消并返回空/角色选择页。
	if (!DBAFrontendStateMachine::IsCharacterCreationState(GetFrontendState()))
	{
		return;
	}
	if (GetFrontendState() == EDBAFrontendState::CharacterCreate_Zodiac)
	{
		CancelCharacterCreation();
		return;
	}

	UDBACharacterCreateDraftSubsystem* Draft = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBACharacterCreateDraftSubsystem>() : nullptr;
	FText Reason;
	if (!Draft || !Draft->Back(Reason))
	{
		UE_LOG(LogDBACharacter, Warning, TEXT("角色创建无法返回上一步：%s"), *Reason.ToString());
		return;
	}

	const EDBAFrontendState TargetState = GetFrontendState() == EDBAFrontendState::CharacterCreate_Element
		? EDBAFrontendState::CharacterCreate_Zodiac
		: GetFrontendState() == EDBAFrontendState::CharacterCreate_FiveCamp
			? EDBAFrontendState::CharacterCreate_Element
			: EDBAFrontendState::CharacterCreate_FiveCamp;
	if (TryTransitionTo(TargetState))
	{
		UpdateLegacyFlowState(TargetState);
	}
}

void UDBAFrontendFlowSubsystem::RequestLogout()
{
	InvalidatePendingFlowRequests();
	const auto CompleteLogout = [this]()
	{
		if (UDBACharacterRosterSubsystem* Roster = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBACharacterRosterSubsystem>() : nullptr)
		{
			Roster->ClearCache();
		}
		CachedCharacters.Reset();
		FrontendSessionContext = FDBAFrontendSessionContext();
		FrontendSessionContext.ClientSessionState = EDBAFrontendState::RecoverableError;
		TryTransitionTo(EDBAFrontendState::Login);
		UpdateLegacyFlowState(EDBAFrontendState::Login);
	};

	if (UDBAOnlineAccountService* AccountService = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDBAOnlineAccountService>() : nullptr)
	{
		AccountService->Logout(FDBAOnLogoutComplete::CreateWeakLambda(this, [CompleteLogout]()
		{
			CompleteLogout();
		}));
		return;
	}

	CompleteLogout();
}

void UDBAFrontendFlowSubsystem::HandleTokenExpired()
{
	InvalidatePendingFlowRequests();
	FrontendSessionContext.AccountId.Reset();
	FrontendSessionContext.ResetCharacterDraft();
	ForceRecoverableErrorAndFallback(EDBAFrontendState::Login, TEXT("登录状态已过期。"));
}

void UDBAFrontendFlowSubsystem::HandleServerUnavailable()
{
	InvalidatePendingFlowRequests();
	const EDBAFrontendState Fallback = CachedCharacters.IsEmpty()
		? EDBAFrontendState::CharacterCreate_Zodiac
		: EDBAFrontendState::CharacterSelect;
	ForceRecoverableErrorAndFallback(Fallback, TEXT("服务器当前不可用。"));
}

void UDBAFrontendFlowSubsystem::RefreshCharacterList()
{
	LoadCharactersAfterLogin();
}

void UDBAFrontendFlowSubsystem::RequestVillageAllocation()
{
	UDBABackendFacadeSubsystem* BackendFacade = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UDBABackendFacadeSubsystem>()
		: nullptr;
	const FDBACharacterId SelectedCharacterId(FrontendSessionContext.SelectedCharacterId);
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
	UE_LOG(LogDBAOnline, Log, TEXT("[DBAFrontendFlowSubsystem] 已请求后台分配新手村服务器。"));
}

void UDBAFrontendFlowSubsystem::HandleVillageAllocationResponse(
	bool bSuccess,
	const FString& ErrorMessage,
	const FString& DataJson)
{
	if (!IsFlowRequestCurrent(PendingVillageRequestGeneration))
	{
		UE_LOG(LogDBAOnline, Verbose, TEXT("[DBAFrontendFlowSubsystem] 忽略已过期的新手村分配回调。"));
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
		const UDBAExternalServiceSettings* Settings = GetDefault<UDBAExternalServiceSettings>();
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

	UE_LOG(LogDBAOnline, Log, TEXT("[DBAFrontendFlowSubsystem] 已发起新手村连接，等待 PlayerState 初始化完成。"));
}

void UDBAFrontendFlowSubsystem::ScheduleVillageConnectionRetry()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		BroadcastErrorAndSetState(TEXT("无法等待大厅服务器：当前世界无效。"), ResolveCharacterFlowState());
		return;
	}

	const UDBAExternalServiceSettings* Settings = GetDefault<UDBAExternalServiceSettings>();
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
	UE_LOG(LogDBAFrontend, Log, TEXT("[DBAFrontendFlowSubsystem] 新手村玩家状态已同步，前端流程完成。"));
}

EDBALoginFlowState UDBAFrontendFlowSubsystem::ResolveCharacterFlowState() const
{
	return CachedCharacters.IsEmpty()
		? EDBALoginFlowState::CharacterCreating
		: EDBALoginFlowState::CharacterSelecting;
}
