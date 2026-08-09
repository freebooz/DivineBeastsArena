// Copyright Freebooz Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "GameDBA/Frontend/ServerDirectory/DBAServerDirectorySubsystem.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAServerDirectoryParsingTest,
	"DBA.Frontend.ServerDirectory.ParsesStableServerIdAndSelectionState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAServerDirectoryParsingTest::RunTest(const FString& Parameters)
{
	const FString Payload = TEXT("[{\"serverId\":\"55000000-0000-0000-0000-000000000001\",\"name\":\"测试维护区\",\"region\":\"cn-east\",\"status\":\"Maintenance\",\"population\":0,\"recommended\":false,\"maintenanceMessage\":\"维护中\",\"canSelect\":false}]");
	TArray<FDBAServerDirectoryEntry> Servers;
	FString Error;

	TestTrue(TEXT("区服目录响应可以解析"), UDBAServerDirectorySubsystem::ParseDirectoryJson(Payload, Servers, Error));
	TestEqual(TEXT("区服目录应保留稳定 ServerId"), Servers[0].ServerId, FString(TEXT("55000000-0000-0000-0000-000000000001")));
	TestEqual(TEXT("维护服必须保留维护状态"), Servers[0].Status, EDBAServerDirectoryStatus::Maintenance);
	TestFalse(TEXT("维护服不可选择"), Servers[0].bCanSelect);
	return true;
}

#endif
