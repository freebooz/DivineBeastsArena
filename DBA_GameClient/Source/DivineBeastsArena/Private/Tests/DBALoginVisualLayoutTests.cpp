// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：登录界面原生兜底布局契约自动化测试。
- 阅读重点：校验参考布局规格、面板最小尺寸与 viewport 缩放下限。
- 修改提示：调整登录布局基准值时同步更新本文件断言。
*/


#if WITH_DEV_AUTOMATION_TESTS

#include "GameDBA/UI/DBAUIFontUtils.h"
#include "GameDBA/Frontend/Auth/UDBALoginFlowWidgetBase.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDBALoginVisualLayoutSpecTest,
	"DivineBeastsArena.UI.Login.ReferenceVisualLayoutSpec",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDBALoginVisualLayoutSpecTest::RunTest(const FString& Parameters)
{
	const FDBALoginVisualLayoutSpec Spec = UDBALoginFlowWidgetBase::GetReferenceVisualLayoutSpec();

	TestEqual(TEXT("登录面板应按参考图居中"), Spec.PanelAnchorX, 0.50f);
	TestEqual(TEXT("登录面板应使用参考图中的中文标题"), Spec.TitleText.ToString(), FString(TEXT("\u795E\u517D\u7ADE\u6280\u573A")));
	TestEqual(TEXT("主操作按钮应匹配参考图"), Spec.PrimaryButtonText.ToString(), FString(TEXT("\u767B\u5F55")));
	TestEqual(TEXT("工具入口数量应匹配参考图"), Spec.LeftToolLabels.Num(), 3);
	TestEqual(TEXT("第一个工具入口应为公告"), Spec.LeftToolLabels[0].ToString(), FString(TEXT("\u516C\u544A")));
	TestEqual(TEXT("第二个工具入口应为客服"), Spec.LeftToolLabels[1].ToString(), FString(TEXT("\u5BA2\u670D")));
	TestEqual(TEXT("第三个工具入口应为修复"), Spec.LeftToolLabels[2].ToString(), FString(TEXT("\u4FEE\u590D")));

	TestEqual(TEXT("参考设计宽度应为 1920"), Spec.ReferenceDesignWidth, 1920.0f);
	TestEqual(TEXT("参考设计高度应为 1080"), Spec.ReferenceDesignHeight, 1080.0f);
	TestTrue(TEXT("登录面板宽度应不小于 860"), Spec.PanelWidth >= 860.0f);
	TestTrue(TEXT("登录面板高度应不小于 600"), Spec.PanelHeight >= 600.0f);
	TestTrue(TEXT("输入行高度应不小于 68"), Spec.InputRowHeight >= 68.0f);
	TestTrue(TEXT("输入框高度应不小于 54"), Spec.InputEditableHeight >= 54.0f);
	TestTrue(TEXT("输入框最小宽度应不小于 560"), Spec.InputMinDesiredWidth >= 560.0f);
	TestTrue(TEXT("紧凑视口宽度阈值应不大于 1280"), Spec.CompactViewportWidthThreshold <= 1280.0f);
	TestTrue(TEXT("紧凑视口高度阈值应不大于 720"), Spec.CompactViewportHeightThreshold <= 720.0f);

	const float Scale960x540 = FMath::Min(960.0f / DBAUIFonts::ReferenceViewportWidth, 540.0f / DBAUIFonts::ReferenceViewportHeight);
	TestEqual(TEXT("960x540 下登录 fit 缩放应为 0.5"), Scale960x540, 0.5f);
	TestTrue(TEXT("960x540 fit 缩放不应被下限抬高"), Scale960x540 <= Spec.MinViewportUIScale);

	const float Scale1920x1080 = FMath::Min(1920.0f / DBAUIFonts::ReferenceViewportWidth, 1080.0f / DBAUIFonts::ReferenceViewportHeight);
	TestEqual(TEXT("1920x1080 下登录缩放应为 1.0"), Scale1920x1080, 1.0f);
	return true;
}

#endif
