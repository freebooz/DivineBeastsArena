// Copyright FreeboozStudio. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "GameCore/Account/DBAAccountServiceBase.h"
#include "GameCore/Account/DBASaveGameVersions.h"
#include "GameCore/Account/DBAOnlineAccountService.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAOnlineAccountFallbackPolicyTest,
	"DivineBeastsArena.GameCore.Account.OnlineService.FallbackPolicy",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAOnlineAccountFallbackPolicyTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("NetworkUnavailable can fallback"), UDBAOnlineAccountService::CanFallbackToMock(EDBAOnlineAccountError::NetworkUnavailable));
	TestTrue(TEXT("Timeout can fallback"), UDBAOnlineAccountService::CanFallbackToMock(EDBAOnlineAccountError::Timeout));
	TestTrue(TEXT("EndpointMissing can fallback"), UDBAOnlineAccountService::CanFallbackToMock(EDBAOnlineAccountError::EndpointMissing));
	TestTrue(TEXT("ServiceUnavailable can fallback"), UDBAOnlineAccountService::CanFallbackToMock(EDBAOnlineAccountError::ServiceUnavailable));
	TestFalse(TEXT("InvalidCredentials cannot fallback"), UDBAOnlineAccountService::CanFallbackToMock(EDBAOnlineAccountError::InvalidCredentials));
	TestFalse(TEXT("AccountUnavailable cannot fallback"), UDBAOnlineAccountService::CanFallbackToMock(EDBAOnlineAccountError::AccountUnavailable));
	TestFalse(TEXT("ValidationFailed cannot fallback"), UDBAOnlineAccountService::CanFallbackToMock(EDBAOnlineAccountError::ValidationFailed));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAAccountScopedSaveSlotNameTest,
	"DivineBeastsArena.GameCore.Account.SaveSlot.ScopedByCommandLineSuffix",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAAccountScopedSaveSlotNameTest::RunTest(const FString& Parameters)
{
	const FString OriginalCommandLine = FCommandLine::Get();
	FCommandLine::Set(TEXT("-DBASaveSlotSuffix=ClientA"));

	TestEqual(
		TEXT("Account slot should include command line suffix"),
		UDBAAccountServiceBase::BuildScopedSaveSlotName(DBASaveGameVersions::SlotNames::ACCOUNT_SLOT),
		FString(DBASaveGameVersions::SlotNames::ACCOUNT_SLOT) + TEXT("_ClientA"));
	TestEqual(
		TEXT("Profile slot should include command line suffix"),
		UDBAAccountServiceBase::BuildScopedSaveSlotName(DBASaveGameVersions::SlotNames::PROFILE_SLOT),
		FString(DBASaveGameVersions::SlotNames::PROFILE_SLOT) + TEXT("_ClientA"));

	FCommandLine::Set(*OriginalCommandLine);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAAccountCommandLineGuestIdentityTest,
	"DivineBeastsArena.GameCore.Account.GuestIdentity.FromCommandLine",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAAccountCommandLineGuestIdentityTest::RunTest(const FString& Parameters)
{
	const FString OriginalCommandLine = FCommandLine::Get();
	FCommandLine::Set(TEXT("-DBAGuestAccountId=LobbyClientB -DBAGuestDisplayName=LobbyBuddy"));

	FDBAAccountInfo AccountInfo = UDBAAccountServiceBase::BuildCommandLineGuestAccountInfo();
	TestEqual(TEXT("Account id should come from command line"), AccountInfo.AccountId.ToString(), FString(TEXT("LobbyClientB")));
	TestEqual(TEXT("Display name should come from command line"), AccountInfo.DisplayName, FString(TEXT("LobbyBuddy")));
	TestEqual(TEXT("Login type should remain Guest"), AccountInfo.LoginType, EDBALoginType::Guest);
	TestTrue(TEXT("Command line guest account should be valid"), AccountInfo.IsValid());

	FCommandLine::Set(*OriginalCommandLine);
	return true;
}

#endif
