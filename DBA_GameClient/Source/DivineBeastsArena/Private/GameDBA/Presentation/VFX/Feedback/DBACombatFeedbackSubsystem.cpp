// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#include "GameDBA/Presentation/VFX/Feedback/DBACombatFeedbackSubsystem.h"

#include "Engine/GameInstance.h"

UDBACombatFeedbackSubsystem::UDBACombatFeedbackSubsystem()
{
	ElementColors.Add(FDBAElementColorPair(EDBAElementType::Fire, FLinearColor(1.0f, 0.27f, 0.0f, 1.0f)));
	ElementColors.Add(FDBAElementColorPair(EDBAElementType::Water, FLinearColor(0.0f, 0.75f, 1.0f, 1.0f)));
	ElementColors.Add(FDBAElementColorPair(EDBAElementType::Metal, FLinearColor(1.0f, 0.84f, 0.0f, 1.0f)));
	ElementColors.Add(FDBAElementColorPair(EDBAElementType::Earth, FLinearColor(0.55f, 0.27f, 0.07f, 1.0f)));
	ElementColors.Add(FDBAElementColorPair(EDBAElementType::Wood, FLinearColor(0.2f, 0.8f, 0.2f, 1.0f)));
}

void UDBACombatFeedbackSubsystem::OnSubsystemInitialize()
{
	// P1-1 改造：项目基类统一调用 Super::Initialize，此处仅执行派生类初始化
	CachedGameInstance = GetGameInstance();
}

void UDBACombatFeedbackSubsystem::OnSubsystemDeinitialize()
{
	// P1-1 改造：项目基类统一调用 Super::Deinitialize，此处仅清理派生类状态
	ClearAllListeners();
	CachedGameInstance.Reset();
}

void UDBACombatFeedbackSubsystem::DispatchCombatEvent(const FDBACombatEventData& EventData)
{
	if (TArray<FOnCombatEvent>* Listeners = GetListenersForTag(EventData.EventTag))
	{
		for (FOnCombatEvent& Delegate : *Listeners)
		{
			Delegate.Broadcast(EventData);
		}
	}
}

void UDBACombatFeedbackSubsystem::ListenForEvent(FGameplayTag EventTag, const FOnCombatEvent& Delegate)
{
	EventListeners.FindOrAdd(EventTag).Add(Delegate);
}

void UDBACombatFeedbackSubsystem::StopListeningForEvent(FGameplayTag EventTag, const FOnCombatEvent& Delegate)
{
	if (TArray<FOnCombatEvent>* Listeners = EventListeners.Find(EventTag))
	{
		Listeners->Empty();
	}
}

FLinearColor UDBACombatFeedbackSubsystem::GetElementColor(EDBAElementType Element) const
{
	switch (Element)
	{
	case EDBAElementType::Fire:
		return FLinearColor(1.0f, 0.27f, 0.0f, 1.0f);
	case EDBAElementType::Water:
		return FLinearColor(0.0f, 0.75f, 1.0f, 1.0f);
	case EDBAElementType::Metal:
		return FLinearColor(1.0f, 0.84f, 0.0f, 1.0f);
	case EDBAElementType::Earth:
		return FLinearColor(0.55f, 0.27f, 0.07f, 1.0f);
	case EDBAElementType::Wood:
		return FLinearColor(0.2f, 0.8f, 0.2f, 1.0f);
	default:
		return FLinearColor::White;
	}
}

TArray<FOnCombatEvent>* UDBACombatFeedbackSubsystem::GetListenersForTag(FGameplayTag Tag)
{
	return EventListeners.Find(Tag);
}

void UDBACombatFeedbackSubsystem::ClearAllListeners()
{
	EventListeners.Empty();
}
