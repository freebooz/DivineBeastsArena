// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#pragma once

#include "CoreMinimal.h"
#include "Fonts/SlateFontInfo.h"

class UWidgetTree;

namespace DBAUIFonts
{
	inline constexpr float ReferenceViewportWidth = 1920.0f;
	inline constexpr float ReferenceViewportHeight = 1080.0f;

	DIVINEBEASTSARENA_API float GetViewportUIScale(const UObject* WorldContextObject);
	DIVINEBEASTSARENA_API float ScaleLayoutValue(const UObject* WorldContextObject, float ReferenceValue);
	DIVINEBEASTSARENA_API FVector2D ScaleVector2D(const UObject* WorldContextObject, const FVector2D& ReferenceValue);
	DIVINEBEASTSARENA_API FMargin ScaleMargin(const UObject* WorldContextObject, float Left, float Top, float Right, float Bottom);
	DIVINEBEASTSARENA_API void ApplyViewportScaledPresentation(class UUserWidget* Widget);
	/** 登录/选角/创建等全屏流程控件：铺满视口且不使用 RenderScale，避免界面不可见或尺寸错误 */
	DIVINEBEASTSARENA_API void ApplyFullscreenFlowViewportPresentation(class UUserWidget* Widget);
	DIVINEBEASTSARENA_API FSlateFontInfo MakeGameFont(float Size, int32 OutlineSize = 1);
	DIVINEBEASTSARENA_API void ApplyGameFontToWidgetTree(UWidgetTree* WidgetTree);
}
