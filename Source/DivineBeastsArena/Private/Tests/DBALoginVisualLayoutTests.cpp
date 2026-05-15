// Copyright Freebooz Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "GameDBA/UI/Lobby/Login/UDBALoginFlowWidgetBase.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBALoginVisualLayoutSpecTest,
	"DivineBeastsArena.UI.Login.ReferenceVisualLayoutSpec",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBALoginVisualLayoutSpecTest::RunTest(const FString& Parameters)
{
	const FDBALoginVisualLayoutSpec Spec = UDBALoginFlowWidgetBase::GetReferenceVisualLayoutSpec();

	TestEqual(TEXT("Panel should be anchored on the right like the reference login art"), Spec.PanelAnchorX, 0.66f);
	TestEqual(TEXT("Panel should use the Chinese title from the reference"), Spec.TitleText.ToString(), FString(TEXT("神兽竞技场")));
	TestEqual(TEXT("Primary CTA should match the reference"), Spec.PrimaryButtonText.ToString(), FString(TEXT("进入游戏")));
	TestEqual(TEXT("Left tool entries should match the reference"), Spec.LeftToolLabels.Num(), 3);
	TestEqual(TEXT("First left tool is announcements"), Spec.LeftToolLabels[0].ToString(), FString(TEXT("公告")));
	TestEqual(TEXT("Second left tool is repair"), Spec.LeftToolLabels[1].ToString(), FString(TEXT("修复")));
	TestEqual(TEXT("Third left tool is support"), Spec.LeftToolLabels[2].ToString(), FString(TEXT("客服")));
	return true;
}

#endif
