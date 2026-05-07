// Copyright FreeboozStudio. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Async/TaskGraphInterfaces.h"
#include "Containers/Ticker.h"
#include "GameCore/Account/DBASaveGameVersions.h"
#include "GameCore/Party/DBAPartyServiceBase.h"
#include "GameCore/Queue/DBAQueueServiceBase.h"
#include "GameCore/Queue/DBAQueueTypes.h"
#include "GameCore/Session/DBAFrontendSessionSubsystem.h"
#include "GameCore/Session/DBALoginFlowSubsystem.h"

namespace
{
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
	UGameplayStatics::DeleteGameInSlot(DBASaveGameVersions::SlotNames::ACCOUNT_SLOT, 0);
	UGameplayStatics::DeleteGameInSlot(DBASaveGameVersions::SlotNames::PROFILE_SLOT, 0);
	UGameplayStatics::DeleteGameInSlot(DBASaveGameVersions::SlotNames::ACCOUNT_SLOT + DBASaveGameVersions::SlotNames::BACKUP_SUFFIX, 0);
	UGameplayStatics::DeleteGameInSlot(DBASaveGameVersions::SlotNames::PROFILE_SLOT + DBASaveGameVersions::SlotNames::BACKUP_SUFFIX, 0);
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
	ResetFlowSaveSlots();

	UGameInstance* GameInstance = ResolveTestGameInstance();
	TestNotNull(TEXT("GameInstance should exist"), GameInstance);
	if (!GameInstance)
	{
		return false;
	}

	UDBALoginFlowSubsystem* LoginFlow = GameInstance->GetSubsystem<UDBALoginFlowSubsystem>();
	UDBAFrontendSessionSubsystem* FrontendSession = GameInstance->GetSubsystem<UDBAFrontendSessionSubsystem>();
	UDBAPartyServiceBase* PartyService = GameInstance->GetSubsystem<UDBAPartyServiceBase>();
	UDBAQueueServiceBase* QueueService = GameInstance->GetSubsystem<UDBAQueueServiceBase>();

	TestNotNull(TEXT("LoginFlow subsystem should exist"), LoginFlow);
	TestNotNull(TEXT("FrontendSession subsystem should exist"), FrontendSession);
	TestNotNull(TEXT("PartyService subsystem should exist"), PartyService);
	TestNotNull(TEXT("QueueService subsystem should exist"), QueueService);
	if (!LoginFlow || !FrontendSession || !PartyService || !QueueService)
	{
		return false;
	}

	LoginFlow->SubmitGuestLogin();
	TestTrue(TEXT("Guest login should enter CharacterCreate on empty role list"), WaitForLoginState(LoginFlow, EDBALoginFlowState::CharacterCreate, FlowTimeoutSeconds));

	FDBACharacterCreateRequest CreateRequest;
	CreateRequest.CharacterName = TEXT("FlowArena_Rat");
	CreateRequest.Zodiac = EDBAZodiac::Rat;
	CreateRequest.PrimaryElement = EDBAElement::Water;
	CreateRequest.FiveCamp = EDBAFiveCamp::East;
	LoginFlow->SubmitCharacterCreation(CreateRequest);

	TestTrue(TEXT("Character creation flow should enter MainLobby"), WaitForLoginState(LoginFlow, EDBALoginFlowState::MainLobby, FlowTimeoutSeconds));
	TestEqual(TEXT("Frontend state should be MainLobby"), FrontendSession->GetCurrentState(), EDBAFrontendSessionState::MainLobby);

	FDBAPartyInfo CreatedParty;
	PartyService->CreateParty(FDBAOnPartyCreated::CreateLambda([&CreatedParty](const FDBAPartyInfo& PartyInfo)
	{
		CreatedParty = PartyInfo;
	}));
	TestTrue(TEXT("CreateParty should produce a valid party"), CreatedParty.IsValid());
	TestEqual(TEXT("Frontend state should be InParty"), FrontendSession->GetCurrentState(), EDBAFrontendSessionState::InParty);

	FDBAQueueInfo QueueInfo;
	QueueService->StartQueue(EDBAQueueType::QuickMatch, FDBAOnQueueStarted::CreateLambda([&QueueInfo](const FDBAQueueInfo& InQueue)
	{
		QueueInfo = InQueue;
	}));
	TestTrue(TEXT("StartQueue should produce a valid queue"), QueueInfo.IsValid());
	TestEqual(TEXT("Frontend state should be InQueue"), FrontendSession->GetCurrentState(), EDBAFrontendSessionState::InQueue);
	TestTrue(TEXT("ReadyCheck should be available"), FrontendSession->GetCurrentReadyCheckInfo().IsValid());

	const FDBAReadyCheckId ReadyCheckId = FrontendSession->GetCurrentReadyCheckInfo().ReadyCheckId;
	bool bReadyConfirmed = false;
	QueueService->ConfirmReady(ReadyCheckId, FDBAOnReadyCheckCompleted::CreateLambda([&bReadyConfirmed](bool bSuccess)
	{
		bReadyConfirmed = bSuccess;
	}));

	TestTrue(TEXT("ConfirmReady should succeed"), bReadyConfirmed);
	TestEqual(TEXT("Frontend state should switch to Loading (entering arena)"), FrontendSession->GetCurrentState(), EDBAFrontendSessionState::Loading);
	TestEqual(TEXT("Match session should switch to Loading"), FrontendSession->GetCurrentMatchSessionInfo().State, EDBAMatchSessionState::Loading);
	return true;
}

#endif

