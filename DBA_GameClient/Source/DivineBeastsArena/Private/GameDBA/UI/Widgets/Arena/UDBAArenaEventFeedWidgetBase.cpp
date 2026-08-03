// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：实现 Arena HUD 事件流 Widget 基类的 C++ 到蓝图转发。
- 阅读重点：事件条目时长在 C++ 层做下限保护，具体呈现由 UMG 蓝图实现。
- 修改提示：该层不保存事件队列状态，避免与蓝图表现层重复维护列表。
*/

#include "GameDBA/UI/Widgets/Arena/UDBAArenaEventFeedWidgetBase.h"
#include "Math/UnrealMathUtility.h"

namespace
{
bool NormalizeEventFeedWidgetText(const FText& Text, FText& OutText)
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

UDBAArenaEventFeedWidgetBase::UDBAArenaEventFeedWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDBAArenaEventFeedWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
}

void UDBAArenaEventFeedWidgetBase::NativeDestruct()
{
	Super::NativeDestruct();
}

void UDBAArenaEventFeedWidgetBase::AddEventEntry(const FText& Text, float Duration)
{
	FText NormalizedText;
	if (!NormalizeEventFeedWidgetText(Text, NormalizedText))
	{
		return;
	}

	BP_OnEventEntryAdded(NormalizedText, FMath::Max(0.0f, Duration));
}

void UDBAArenaEventFeedWidgetBase::ClearEventFeed()
{
	BP_OnEventFeedCleared();
}
