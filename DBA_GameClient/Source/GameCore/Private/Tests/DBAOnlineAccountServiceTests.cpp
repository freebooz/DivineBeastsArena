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
	TestTrue(TEXT("网络不可用应允许兜底"), UDBAOnlineAccountService::CanFallbackToMock(EDBAOnlineAccountError::NetworkUnavailable));
	TestTrue(TEXT("超时应允许兜底"), UDBAOnlineAccountService::CanFallbackToMock(EDBAOnlineAccountError::Timeout));
	TestFalse(TEXT("端点缺失应暴露后端契约漂移而不是兜底"), UDBAOnlineAccountService::CanFallbackToMock(EDBAOnlineAccountError::EndpointMissing));
	TestTrue(TEXT("服务不可用应允许兜底"), UDBAOnlineAccountService::CanFallbackToMock(EDBAOnlineAccountError::ServiceUnavailable));
	TestFalse(TEXT("凭据无效不应兜底"), UDBAOnlineAccountService::CanFallbackToMock(EDBAOnlineAccountError::InvalidCredentials));
	TestFalse(TEXT("账号不可用不应兜底"), UDBAOnlineAccountService::CanFallbackToMock(EDBAOnlineAccountError::AccountUnavailable));
	TestFalse(TEXT("校验失败不应兜底"), UDBAOnlineAccountService::CanFallbackToMock(EDBAOnlineAccountError::ValidationFailed));
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
		TEXT("账号存档槽应包含命令行后缀"),
		UDBAAccountServiceBase::BuildScopedSaveSlotName(DBASaveGameVersions::SlotNames::ACCOUNT_SLOT),
		FString(DBASaveGameVersions::SlotNames::ACCOUNT_SLOT) + TEXT("_ClientA"));
	TestEqual(
		TEXT("档案存档槽应包含命令行后缀"),
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
	TestEqual(TEXT("账号标识应来自命令行"), AccountInfo.AccountId.ToString(), FString(TEXT("LobbyClientB")));
	TestEqual(TEXT("显示名称应来自命令行"), AccountInfo.DisplayName, FString(TEXT("LobbyBuddy")));
	TestEqual(TEXT("登录类型应保持游客"), AccountInfo.LoginType, EDBALoginType::Guest);
	TestTrue(TEXT("命令行游客账号应有效"), AccountInfo.IsValid());

	FCommandLine::Set(*OriginalCommandLine);
	return true;
}

#endif
