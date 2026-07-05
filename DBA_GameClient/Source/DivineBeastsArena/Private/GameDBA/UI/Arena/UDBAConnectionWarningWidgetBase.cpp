// Copyright FreeboozStudio Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/UI/Arena/UDBAConnectionWarningWidgetBase.h"

namespace
{
	bool NormalizeConnectionWarningText(const FText& Text, FText& OutText)
	{
		const FString NormalizedText = Text.ToString().TrimStartAndEnd();
		if (NormalizedText.IsEmpty())
		{
			return false;
		}

		OutText = FText::FromString(NormalizedText);
		return true;
	}
}

UDBAConnectionWarningWidgetBase::UDBAConnectionWarningWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDBAConnectionWarningWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
	if (bCachedWarningVisible && !CachedWarningMessage.IsEmpty())
	{
		BP_OnWarningShown(CachedWarningMessage);
	}
	else
	{
		BP_OnWarningHidden();
	}
}

void UDBAConnectionWarningWidgetBase::NativeDestruct()
{
	Super::NativeDestruct();
}

void UDBAConnectionWarningWidgetBase::ShowWarning(const FText& Message)
{
	FText NormalizedMessage;
	if (!NormalizeConnectionWarningText(Message, NormalizedMessage))
	{
		return;
	}

	CachedWarningMessage = NormalizedMessage;
	bCachedWarningVisible = true;
	BP_OnWarningShown(CachedWarningMessage);
}

void UDBAConnectionWarningWidgetBase::HideWarning()
{
	CachedWarningMessage = FText::GetEmpty();
	bCachedWarningVisible = false;
	BP_OnWarningHidden();
}
