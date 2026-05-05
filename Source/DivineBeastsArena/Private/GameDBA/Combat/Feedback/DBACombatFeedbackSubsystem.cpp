// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Combat/Feedback/DBACombatFeedbackSubsystem.h"
#include "Engine/World.h"
#include "GameInstance.h"

UDBACombatFeedbackSubsystem::UDBACombatFeedbackSubsystem()
{
	// 默认元素颜色配置
	// 注意: EDBAElementType 包含 Metal, Wood, Water, Fire, Earth
	// Ice 和 Lightning 暂用 Water 和 Metal 代替，后续可扩展枚举
	ElementColors.Add(FDBAElementColorPair(EDBAElementType::Fire, FLinearColor(1.0f, 0.27f, 0.0f, 1.0f)));      // #FF4500 橙红色
	ElementColors.Add(FDBAElementColorPair(EDBAElementType::Water, FLinearColor(0.0f, 0.75f, 1.0f, 1.0f)));     // #00BFFF 冰蓝色 (Water代Ice)
	ElementColors.Add(FDBAElementColorPair(EDBAElementType::Metal, FLinearColor(1.0f, 0.84f, 0.0f, 1.0f)));     // #FFD700 金黄色 (Metal代Lightning)
	ElementColors.Add(FDBAElementColorPair(EDBAElementType::Earth, FLinearColor(0.55f, 0.27f, 0.07f, 1.0f)));  // #8B4513 棕色
	ElementColors.Add(FDBAElementColorPair(EDBAElementType::Wood, FLinearColor(0.2f, 0.8f, 0.2f, 1.0f)));      // #32CD32 绿色
}

void UDBACombatFeedbackSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	CachedGameInstance = GetGameInstance();
}

void UDBACombatFeedbackSubsystem::Deinitialize()
{
	ClearAllListeners();
	CachedGameInstance.Reset();
	Super::Deinitialize();
}

void UDBACombatFeedbackSubsystem::DispatchCombatEvent(const FDBACombatEventData& EventData)
{
	// 广播给所有监听该标签的委托
	TArray<FOnCombatEvent*>* Listeners = GetListenersForTag(EventData.EventTag);
	if (Listeners)
	{
		for (FOnCombatEvent* Delegate : *Listeners)
		{
			if (Delegate)
			{
				Delegate->Broadcast(EventData);
			}
		}
	}
}

void UDBACombatFeedbackSubsystem::ListenForEvent(FGameplayTag EventTag, const FOnCombatEvent& Delegate)
{
	TArray<FOnCombatEvent>& Listeners = EventListeners.FindOrAdd(EventTag);
	Listeners.Add(Delegate);
}

void UDBACombatFeedbackSubsystem::StopListeningForEvent(FGameplayTag EventTag, const FOnCombatEvent& Delegate)
{
	if (TArray<FOnCombatEvent>* Listeners = EventListeners.Find(EventTag))
	{
		Listeners->Remove(Delegate);
	}
}

FLinearColor UDBACombatFeedbackSubsystem::GetElementColor(EDBAElementType Element)
{
	// 先从配置中查找
	for (const FDBAElementColorPair& Pair : ElementColors)
	{
		if (Pair.Element == Element)
		{
			return Pair.Color;
		}
	}

	// 默认返回白色
	return FLinearColor::White;
}

TArray<FOnCombatEvent*>* UDBACombatFeedbackSubsystem::GetListenersForTag(FGameplayTag Tag)
{
	if (TArray<FOnCombatEvent>* Listeners = EventListeners.Find(Tag))
	{
		return Listeners;
	}
	return nullptr;
}

void UDBACombatFeedbackSubsystem::ClearAllListeners()
{
	EventListeners.Empty();
}