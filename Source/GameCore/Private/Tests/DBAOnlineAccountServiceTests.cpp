// Copyright FreeboozStudio. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
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

#endif
