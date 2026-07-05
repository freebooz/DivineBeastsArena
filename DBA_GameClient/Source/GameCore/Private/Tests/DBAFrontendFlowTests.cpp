// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Async/TaskGraphInterfaces.h"
#include "Containers/Ticker.h"
#include "GameCore/Account/DBAAccountServiceBase.h"
#include "GameCore/Account/DBAOnlineAccountService.h"
#include "GameCore/Account/DBASaveGameVersions.h"
#include "GameCore/Party/DBAPartyServiceBase.h"
#include "GameCore/Queue/DBAQueueServiceBase.h"
#include "GameCore/Queue/DBAQueueTypes.h"
#include "GameCore/Session/DBAFrontendSessionSubsystem.h"
#include "GameCore/Session/DBALoginFlowSubsystem.h"

namespace
{
class FScopedFlowCommandLine
{
public:
	explicit FScopedFlowCommandLine(const TCHAR* ExtraFlags)
		: OriginalCommandLine(FCommandLine::Get())
	{
		const FString NewCommandLine = FString::Printf(TEXT("%s %s"), *OriginalCommandLine, ExtraFlags);
		FCommandLine::Set(*NewCommandLine);
	}

	~FScopedFlowCommandLine()
	{
		FCommandLine::Set(*OriginalCommandLine);
	}

private:
	FString OriginalCommandLine;
};

UGameInstance* ResolveTestGameInstance()
{
	static TObjectPtr<UGameInstance> StandaloneGameInstance = nullptr;
	if (StandaloneGameInstance)
	{
		return StandaloneGameInstance;
	}

	if (!GEngine)
	{
		return nullptr;
	}

	StandaloneGameInstance = NewObject<UGameInstance>(GEngine);
	if (!StandaloneGameInstance)
	{
		return nullptr;
	}

	StandaloneGameInstance->InitializeStandalone();
	return StandaloneGameInstance;
}

void ResetFlowSaveSlots()
{
	const FString AccountSlot = UDBAAccountServiceBase::BuildScopedSaveSlotName(DBASaveGameVersions::SlotNames::ACCOUNT_SLOT);
	const FString ProfileSlot = UDBAAccountServiceBase::BuildScopedSaveSlotName(DBASaveGameVersions::SlotNames::PROFILE_SLOT);
	UGameplayStatics::DeleteGameInSlot(AccountSlot, 0);
	UGameplayStatics::DeleteGameInSlot(ProfileSlot, 0);
	UGameplayStatics::DeleteGameInSlot(AccountSlot + DBASaveGameVersions::SlotNames::BACKUP_SUFFIX, 0);
	UGameplayStatics::DeleteGameInSlot(ProfileSlot + DBASaveGameVersions::SlotNames::BACKUP_SUFFIX, 0);
}

bool WaitForLoginState(UDBALoginFlowSubsystem* LoginFlow, EDBALoginFlowState ExpectedState, double TimeoutSeconds)
{
	if (!LoginFlow)
	{
		return false;
	}

	const double Start = FPlatformTime::Seconds();
	while (FPlatformTime::Seconds() - Start <= TimeoutSeconds)
	{
		if (LoginFlow->GetFlowState() == ExpectedState)
		{
			return true;
		}

		FTaskGraphInterface::Get().ProcessThreadUntilIdle(ENamedThreads::GameThread);
		FTSTicker::GetCoreTicker().Tick(0.01f);
		FPlatformProcess::Sleep(0.01f);
	}

	return LoginFlow->GetFlowState() == ExpectedState;
}

constexpr double FlowTimeoutSeconds = 12.0;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAFrontendFullFlowTest,
	"DivineBeastsArena.GameCore.Session.FrontendFlow.LoginPartyQueueArena",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAFrontendFullFlowTest::RunTest(const FString& Parameters)
{
	FScopedFlowCommandLine CommandLine(TEXT("-DBAForceMockAccount -DBASaveSlotSuffix=FrontendFullFlow"));
	ResetFlowSaveSlots();

	UGameInstance* GameInstance = ResolveTestGameInstance();
	TestNotNull(TEXT("游戏实例应存在"), GameInstance);
	if (!GameInstance)
	{
		return false;
	}

	UDBALoginFlowSubsystem* LoginFlow = GameInstance->GetSubsystem<UDBALoginFlowSubsystem>();
	UDBAOnlineAccountService* AccountService = GameInstance->GetSubsystem<UDBAOnlineAccountService>();
	UDBAFrontendSessionSubsystem* FrontendSession = GameInstance->GetSubsystem<UDBAFrontendSessionSubsystem>();
	UDBAPartyServiceBase* PartyService = GameInstance->GetSubsystem<UDBAPartyServiceBase>();
	UDBAQueueServiceBase* QueueService = GameInstance->GetSubsystem<UDBAQueueServiceBase>();

	TestNotNull(TEXT("登录流程子系统应存在"), LoginFlow);
	TestNotNull(TEXT("账号服务子系统应存在"), AccountService);
	TestNotNull(TEXT("前端会话子系统应存在"), FrontendSession);
	TestNotNull(TEXT("队伍服务子系统应存在"), PartyService);
	TestNotNull(TEXT("队列服务子系统应存在"), QueueService);
	if (!LoginFlow || !AccountService || !FrontendSession || !PartyService || !QueueService)
	{
		return false;
	}

	AccountService->Logout(FDBAOnLogoutComplete());
	FrontendSession->ResetSession();

	LoginFlow->SubmitGuestLogin();
	TestTrue(TEXT("游客登录在角色列表为空时应进入角色创建"), WaitForLoginState(LoginFlow, EDBALoginFlowState::CharacterCreate, FlowTimeoutSeconds));

	FDBACharacterCreateRequest CreateRequest;
	CreateRequest.CharacterName = TEXT("FlowArena_Rat");
	CreateRequest.Zodiac = EDBAZodiac::Rat;
	CreateRequest.PrimaryElement = EDBAElement::Water;
	CreateRequest.FiveCamp = EDBAFiveCamp::East;
	LoginFlow->SubmitCharacterCreation(CreateRequest);

	TestTrue(TEXT("角色创建流程应进入主大厅"), WaitForLoginState(LoginFlow, EDBALoginFlowState::MainLobby, FlowTimeoutSeconds));
	TestEqual(TEXT("前端状态应为主大厅"), FrontendSession->GetCurrentState(), EDBAFrontendSessionState::MainLobby);

	FDBAPartyInfo CreatedParty;
	PartyService->CreateParty(FDBAOnPartyCreated::CreateLambda([&CreatedParty](const FDBAPartyInfo& PartyInfo)
	{
		CreatedParty = PartyInfo;
	}));
	TestTrue(TEXT("创建队伍应产出有效队伍"), CreatedParty.IsValid());
	TestEqual(TEXT("前端状态应为已组队"), FrontendSession->GetCurrentState(), EDBAFrontendSessionState::InParty);

	FDBAQueueInfo QueueInfo;
	QueueService->StartQueue(EDBAQueueType::QuickMatch, FDBAOnQueueStarted::CreateLambda([&QueueInfo](const FDBAQueueInfo& InQueue)
	{
		QueueInfo = InQueue;
	}));
	TestTrue(TEXT("开始队列应产出有效队列"), QueueInfo.IsValid());
	TestEqual(TEXT("前端状态应为排队中"), FrontendSession->GetCurrentState(), EDBAFrontendSessionState::InQueue);
	TestTrue(TEXT("准备确认应可用"), FrontendSession->GetCurrentReadyCheckInfo().IsValid());

	const FDBAReadyCheckId ReadyCheckId = FrontendSession->GetCurrentReadyCheckInfo().ReadyCheckId;
	bool bReadyConfirmed = false;
	QueueService->ConfirmReady(ReadyCheckId, FDBAOnReadyCheckCompleted::CreateLambda([&bReadyConfirmed](bool bSuccess)
	{
		bReadyConfirmed = bSuccess;
	}));

	TestTrue(TEXT("确认准备应成功"), bReadyConfirmed);
	TestEqual(TEXT("前端状态应切换为加载中并进入竞技场"), FrontendSession->GetCurrentState(), EDBAFrontendSessionState::Loading);
	TestEqual(TEXT("比赛会话应切换为加载中"), FrontendSession->GetCurrentMatchSessionInfo().State, EDBAMatchSessionState::Loading);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAFrontendTravelContextBuildSummaryGateTest,
	"DivineBeastsArena.GameCore.Session.FrontendFlow.TravelContextRejectsInvalidBuildSummary",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAFrontendTravelContextBuildSummaryGateTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = ResolveTestGameInstance();
	TestNotNull(TEXT("游戏实例应存在"), GameInstance);
	if (!GameInstance)
	{
		return false;
	}

	UDBAFrontendSessionSubsystem* FrontendSession = GameInstance->GetSubsystem<UDBAFrontendSessionSubsystem>();
	TestNotNull(TEXT("前端会话子系统应存在"), FrontendSession);
	if (!FrontendSession)
	{
		return false;
	}

	FrontendSession->ResetSession();

	FDBATravelContext ValidContext;
	ValidContext.MatchSessionId = FDBAMatchSessionId(TEXT("match_valid_build"));
	ValidContext.AccountId = FDBAAccountId(TEXT("account_valid_build"));
	ValidContext.CharacterId = FDBACharacterId(TEXT("character_valid_build"));
	ValidContext.TeamId = 1;
	ValidContext.SelectedZodiac = EDBAZodiac::Rat;
	ValidContext.SelectedElement = EDBAElement::Water;
	ValidContext.SelectedFiveCamp = EDBAFiveCamp::East;
	ValidContext.FixedSkillGroupId = FName(TEXT("Rat_Water"));
	ValidContext.MapName = TEXT("/Game/Maps/Lobby/LobbyMap");
	ValidContext.ServerAddress = TEXT("127.0.0.1");
	ValidContext.ServerPort = 17777;
	ValidContext.SessionToken = TEXT("session_valid_build");

	TestTrue(TEXT("有效旅行上下文应被接受"), FrontendSession->TrySetCurrentTravelContext(ValidContext));
	TestEqual(TEXT("有效旅行上下文应被保存"), FrontendSession->GetCurrentTravelContext().FixedSkillGroupId, FName(TEXT("Rat_Water")));
	TestEqual(TEXT("已接受旅行上下文应进入加载中"), FrontendSession->GetCurrentState(), EDBAFrontendSessionState::Loading);

	FDBATravelContext TamperedContext = ValidContext;
	TamperedContext.MatchSessionId = FDBAMatchSessionId(TEXT("match_tampered_build"));
	TamperedContext.FixedSkillGroupId = FName(TEXT("Rat_Fire"));

	TestFalse(TEXT("被篡改旅行上下文应被拒绝"), FrontendSession->TrySetCurrentTravelContext(TamperedContext));
	TestEqual(TEXT("被拒绝旅行上下文不得替换已保存固定技能组标识"), FrontendSession->GetCurrentTravelContext().FixedSkillGroupId, FName(TEXT("Rat_Water")));
	TestEqual(TEXT("被拒绝旅行上下文不得替换已保存比赛会话标识"), FrontendSession->GetCurrentTravelContext().MatchSessionId, FDBAMatchSessionId(TEXT("match_valid_build")));
	TestEqual(TEXT("被拒绝旅行上下文应保持已接受上下文的加载中状态"), FrontendSession->GetCurrentState(), EDBAFrontendSessionState::Loading);
	return true;
}

#endif
