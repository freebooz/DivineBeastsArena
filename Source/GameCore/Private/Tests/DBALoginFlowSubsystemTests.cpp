// Copyright FreeboozStudio. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Containers/Ticker.h"
#include "Async/TaskGraphInterfaces.h"
#include "Kismet/GameplayStatics.h"
#include "GameCore/Account/DBAOnlineAccountService.h"
#include "GameCore/Account/DBASaveGameVersions.h"
#include "GameCore/Session/DBALoginFlowSubsystem.h"
#include "GameCore/Session/DBAFrontendSessionSubsystem.h"

namespace
{
UGameInstance* ResolveGameInstance()
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

void ResetLoginRelatedSaveSlots()
{
	UGameplayStatics::DeleteGameInSlot(DBASaveGameVersions::SlotNames::ACCOUNT_SLOT, 0);
	UGameplayStatics::DeleteGameInSlot(DBASaveGameVersions::SlotNames::PROFILE_SLOT, 0);
	UGameplayStatics::DeleteGameInSlot(DBASaveGameVersions::SlotNames::ACCOUNT_SLOT + DBASaveGameVersions::SlotNames::BACKUP_SUFFIX, 0);
	UGameplayStatics::DeleteGameInSlot(DBASaveGameVersions::SlotNames::PROFILE_SLOT + DBASaveGameVersions::SlotNames::BACKUP_SUFFIX, 0);
}

bool WaitForFlowState(UDBALoginFlowSubsystem* LoginFlow, EDBALoginFlowState ExpectedState, double TimeoutSeconds)
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

constexpr double LoginFlowAsyncTimeoutSeconds = 12.0;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBALoginFlowTransitionTest,
	"DivineBeastsArena.GameCore.Session.LoginFlow.Transitions",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBALoginFlowTransitionTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("Empty characters should require creation"), UDBALoginFlowSubsystem::ShouldEnterCharacterCreate(0));
	TestTrue(TEXT("Negative character count should require creation"), UDBALoginFlowSubsystem::ShouldEnterCharacterCreate(-1));
	TestFalse(TEXT("Existing characters should not require creation"), UDBALoginFlowSubsystem::ShouldEnterCharacterCreate(1));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBALoginFlowGuestCreateToLobbyTest,
	"DivineBeastsArena.GameCore.Session.LoginFlow.GuestCreateToLobby",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBALoginFlowGuestCreateToLobbyTest::RunTest(const FString& Parameters)
{
	ResetLoginRelatedSaveSlots();

	UGameInstance* GameInstance = ResolveGameInstance();
	TestNotNull(TEXT("GameInstance should exist"), GameInstance);
	if (!GameInstance)
	{
		return false;
	}

	UDBALoginFlowSubsystem* LoginFlow = GameInstance->GetSubsystem<UDBALoginFlowSubsystem>();
	UDBAOnlineAccountService* AccountService = GameInstance->GetSubsystem<UDBAOnlineAccountService>();
	UDBAFrontendSessionSubsystem* FrontendSession = GameInstance->GetSubsystem<UDBAFrontendSessionSubsystem>();

	TestNotNull(TEXT("LoginFlow subsystem should exist"), LoginFlow);
	TestNotNull(TEXT("AccountService subsystem should exist"), AccountService);
	TestNotNull(TEXT("FrontendSession subsystem should exist"), FrontendSession);
	if (!LoginFlow || !AccountService || !FrontendSession)
	{
		return false;
	}

	LoginFlow->SubmitGuestLogin();
	TestTrue(TEXT("Guest login without role should enter CharacterCreate"), WaitForFlowState(LoginFlow, EDBALoginFlowState::CharacterCreate, LoginFlowAsyncTimeoutSeconds));

	FDBACharacterCreateRequest CreateRequest;
	CreateRequest.CharacterName = TEXT("AutoFlow_Rat");
	CreateRequest.Zodiac = EDBAZodiac::Rat;
	CreateRequest.PrimaryElement = EDBAElement::Water;
	CreateRequest.FiveCamp = EDBAFiveCamp::East;
	LoginFlow->SubmitCharacterCreation(CreateRequest);

	TestTrue(TEXT("After character creation + selection flow should enter MainLobby"), WaitForFlowState(LoginFlow, EDBALoginFlowState::MainLobby, LoginFlowAsyncTimeoutSeconds));
	TestEqual(TEXT("Frontend session should be MainLobby"), FrontendSession->GetCurrentState(), EDBAFrontendSessionState::MainLobby);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBALoginFlowGuestSelectToLobbyTest,
	"DivineBeastsArena.GameCore.Session.LoginFlow.GuestSelectToLobby",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBALoginFlowGuestSelectToLobbyTest::RunTest(const FString& Parameters)
{
	ResetLoginRelatedSaveSlots();

	UGameInstance* GameInstance = ResolveGameInstance();
	TestNotNull(TEXT("GameInstance should exist"), GameInstance);
	if (!GameInstance)
	{
		return false;
	}

	UDBALoginFlowSubsystem* LoginFlow = GameInstance->GetSubsystem<UDBALoginFlowSubsystem>();
	UDBAOnlineAccountService* AccountService = GameInstance->GetSubsystem<UDBAOnlineAccountService>();
	UDBAFrontendSessionSubsystem* FrontendSession = GameInstance->GetSubsystem<UDBAFrontendSessionSubsystem>();

	TestNotNull(TEXT("LoginFlow subsystem should exist"), LoginFlow);
	TestNotNull(TEXT("AccountService subsystem should exist"), AccountService);
	TestNotNull(TEXT("FrontendSession subsystem should exist"), FrontendSession);
	if (!LoginFlow || !AccountService || !FrontendSession)
	{
		return false;
	}

	LoginFlow->SubmitGuestLogin();
	FDBACharacterCreateRequest CreateRequest;
	CreateRequest.CharacterName = TEXT("AutoFlow_Ox");
	CreateRequest.Zodiac = EDBAZodiac::Ox;
	CreateRequest.PrimaryElement = EDBAElement::Earth;
	CreateRequest.FiveCamp = EDBAFiveCamp::Center;
	LoginFlow->SubmitCharacterCreation(CreateRequest);
	TestTrue(TEXT("Preparation: first create should reach MainLobby"), WaitForFlowState(LoginFlow, EDBALoginFlowState::MainLobby, LoginFlowAsyncTimeoutSeconds));

	LoginFlow->SubmitGuestLogin();
	TestTrue(TEXT("Guest login with existing role should enter CharacterSelect"), WaitForFlowState(LoginFlow, EDBALoginFlowState::CharacterSelect, LoginFlowAsyncTimeoutSeconds));

	const TArray<FDBACharacterSummary>& Characters = LoginFlow->GetCachedCharacters();
	TestTrue(TEXT("Character list should contain at least one role"), Characters.Num() > 0);
	if (Characters.Num() <= 0)
	{
		return false;
	}

	LoginFlow->SubmitCharacterSelection(Characters[0].CharacterId);
	TestTrue(TEXT("After selection flow should enter MainLobby"), WaitForFlowState(LoginFlow, EDBALoginFlowState::MainLobby, LoginFlowAsyncTimeoutSeconds));
	TestEqual(TEXT("Frontend session should be MainLobby"), FrontendSession->GetCurrentState(), EDBAFrontendSessionState::MainLobby);
	return true;
}

#endif
