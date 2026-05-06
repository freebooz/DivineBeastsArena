// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Combat/Feedback/DBACombatFeedbackSubsystem.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

UDBACombatFeedbackSubsystem::UDBACombatFeedbackSubsystem()
{
	ElementColors.Add(FDBAElementColorPair(EDBAElementType::Fire, FLinearColor(1.0f, 0.27f, 0.0f, 1.0f)));
	ElementColors.Add(FDBAElementColorPair(EDBAElementType::Water, FLinearColor(0.0f, 0.75f, 1.0f, 1.0f)));
	ElementColors.Add(FDBAElementColorPair(EDBAElementType::Metal, FLinearColor(1.0f, 0.84f, 0.0f, 1.0f)));
	ElementColors.Add(FDBAElementColorPair(EDBAElementType::Earth, FLinearColor(0.55f, 0.27f, 0.07f, 1.0f)));
	ElementColors.Add(FDBAElementColorPair(EDBAElementType::Wood, FLinearColor(0.2f, 0.8f, 0.2f, 1.0f)));
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
	TArray<FOnCombatEvent>* Listeners = GetListenersForTag(EventData.EventTag);
	if (Listeners)
	{
		for (FOnCombatEvent& Delegate : *Listeners)
		{
			Delegate.Broadcast(EventData);
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
	(void)Delegate;
	(void)EventTag;
}

FLinearColor UDBACombatFeedbackSubsystem::GetElementColor(EDBAElementType Element) const
{
	for (const FDBAElementColorPair& Pair : ElementColors)
	{
		if (Pair.Element == Element)
		{
			return Pair.Color;
		}
	}

	return FLinearColor::White;
}

TArray<FOnCombatEvent>* UDBACombatFeedbackSubsystem::GetListenersForTag(FGameplayTag Tag)
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
