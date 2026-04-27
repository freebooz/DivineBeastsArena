// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Frontend/DBAInteractionPromptWidgetBase.h"

UDBAInteractionPromptWidgetBase::UDBAInteractionPromptWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, InteractionType(EDBAInteractionType::None)
	, CachedCanInteract(false)
	, InteractionProgress(0.0f)
{
}

void UDBAInteractionPromptWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UDBAInteractionPromptWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
}

void UDBAInteractionPromptWidgetBase::NativeDestruct()
{
	Super::NativeDestruct();
}

void UDBAInteractionPromptWidgetBase::ShowPrompt(EDBAInteractionType Type, const FText& InCachedObjectName, const FText& InCachedPromptText, bool bInCanInteract)
{
	InteractionType = Type;
	CachedObjectName = InCachedObjectName;
	CachedPromptText = InCachedPromptText;
	CachedCanInteract = bInCanInteract;
	InteractionProgress = 0.0f;

	BP_OnShowPrompt(Type, InCachedObjectName, InCachedPromptText, bInCanInteract);
}

void UDBAInteractionPromptWidgetBase::HidePrompt()
{
	InteractionType = EDBAInteractionType::None;
	InteractionProgress = 0.0f;

	BP_OnHidePrompt();
}

void UDBAInteractionPromptWidgetBase::UpdateInteractionProgress(float Progress)
{
	InteractionProgress = FMath::Clamp(Progress, 0.0f, 1.0f);
	BP_OnUpdateProgress(InteractionProgress);
}
