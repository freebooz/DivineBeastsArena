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

	TestEqual(TEXT("Panel should be centered like the approved login art"), Spec.PanelAnchorX, 0.50f);
	TestEqual(TEXT("Panel should use the Chinese title from the reference"), Spec.TitleText.ToString(), FString(TEXT("\u795E\u517D\u7ADE\u6280\u573A")));
	TestEqual(TEXT("Primary CTA should match the reference"), Spec.PrimaryButtonText.ToString(), FString(TEXT("\u767B\u5F55")));
	TestEqual(TEXT("Tool entries should match the reference"), Spec.LeftToolLabels.Num(), 3);
	TestEqual(TEXT("First tool is announcements"), Spec.LeftToolLabels[0].ToString(), FString(TEXT("\u516C\u544A")));
	TestEqual(TEXT("Second tool is support"), Spec.LeftToolLabels[1].ToString(), FString(TEXT("\u5BA2\u670D")));
	TestEqual(TEXT("Third tool is repair"), Spec.LeftToolLabels[2].ToString(), FString(TEXT("\u4FEE\u590D")));
	return true;
}

#endif
