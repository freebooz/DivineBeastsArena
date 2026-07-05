// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


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

	TestEqual(TEXT("登录面板应按参考图居中"), Spec.PanelAnchorX, 0.50f);
	TestEqual(TEXT("登录面板应使用参考图中的中文标题"), Spec.TitleText.ToString(), FString(TEXT("\u795E\u517D\u7ADE\u6280\u573A")));
	TestEqual(TEXT("主操作按钮应匹配参考图"), Spec.PrimaryButtonText.ToString(), FString(TEXT("\u767B\u5F55")));
	TestEqual(TEXT("工具入口数量应匹配参考图"), Spec.LeftToolLabels.Num(), 3);
	TestEqual(TEXT("第一个工具入口应为公告"), Spec.LeftToolLabels[0].ToString(), FString(TEXT("\u516C\u544A")));
	TestEqual(TEXT("第二个工具入口应为客服"), Spec.LeftToolLabels[1].ToString(), FString(TEXT("\u5BA2\u670D")));
	TestEqual(TEXT("第三个工具入口应为修复"), Spec.LeftToolLabels[2].ToString(), FString(TEXT("\u4FEE\u590D")));
	return true;
}

#endif
