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
