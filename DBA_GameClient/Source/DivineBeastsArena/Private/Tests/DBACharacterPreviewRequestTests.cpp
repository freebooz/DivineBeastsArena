// Copyright Freebooz Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "GameDBA/Frontend/Preview/DBACharacterPreviewSubsystem.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDBACharacterPreviewRequestRaceTest, "DBA.Frontend.Preview.AsyncRequestGeneration", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBACharacterPreviewRequestRaceTest::RunTest(const FString& Parameters)
{
	FDBACharacterPreviewRequestGate Gate;
	const uint32 RatRequest = Gate.BeginRequest();
	const uint32 TigerRequest = Gate.BeginRequest();
	const uint32 DragonRequest = Gate.BeginRequest();

	TestFalse(TEXT("鼠请求在虎、龙后不得写入预览"), Gate.IsCurrent(RatRequest));
	TestFalse(TEXT("虎请求在龙后不得写入预览"), Gate.IsCurrent(TigerRequest));
	TestTrue(TEXT("最后的龙请求可以写入预览"), Gate.IsCurrent(DragonRequest));
	Gate.Invalidate();
	TestFalse(TEXT("离开角色页面后旧请求不得写入预览"), Gate.IsCurrent(DragonRequest));
	return true;
}

#endif
