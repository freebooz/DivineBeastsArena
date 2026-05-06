// Copyright FreeboozStudio. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "GameCore/Session/DBALoginFlowSubsystem.h"

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

#endif
