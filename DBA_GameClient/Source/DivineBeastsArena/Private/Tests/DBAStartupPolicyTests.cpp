// Copyright Freebooz Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "GameDBA/Frontend/Startup/DBAStartupPolicy.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBAStartupPolicyTests,
	"DBA.Frontend.Startup.Policy",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBAStartupPolicyTests::RunTest(const FString& Parameters)
{
	const FSoftObjectPath ValidFrontendMap(TEXT("/Game/Maps/Frontend/L_DBA_Frontend.L_DBA_Frontend"));
	TestTrue(TEXT("冷启动：有效前台地图配置可进入前台"), DBAStartupPolicy::IsFrontendMapConfigurationValid(ValidFrontendMap));
	TestEqual(
		TEXT("无网络：配置有效时进入前台并展示可恢复状态"),
		DBAStartupPolicy::ResolveBackendCheck(true, false),
		EDBAStartupCheckDisposition::TravelToFrontend);
	TestEqual(
		TEXT("配置缺失：阻断旅行并进入致命错误"),
		DBAStartupPolicy::ResolveBackendCheck(false, true),
		EDBAStartupCheckDisposition::FatalFailure);
	return true;
}

#endif
