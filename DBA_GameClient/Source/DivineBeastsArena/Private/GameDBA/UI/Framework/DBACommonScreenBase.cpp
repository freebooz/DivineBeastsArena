// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Framework/DBACommonScreenBase.h"

#include "Components/Widget.h"
#include "Input/Events.h"
#include "InputCoreTypes.h"

UDBACommonScreenBase::UDBACommonScreenBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true);
}

void UDBACommonScreenBase::NativeConstruct()
{
	Super::NativeConstruct();
	if (PreferredFocusWidget)
	{
		PreferredFocusWidget->SetKeyboardFocus();
	}
}

FReply UDBACommonScreenBase::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey& Key = InKeyEvent.GetKey();
	if (bConsumesBackInput && (Key == EKeys::Escape || Key == EKeys::Gamepad_Special_Right))
	{
		RequestBack();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UDBACommonScreenBase::RequestBack()
{
	OnBackRequested.Broadcast();
}

void UDBACommonScreenBase::SetPreferredFocus(UWidget* InWidget)
{
	PreferredFocusWidget = InWidget;
	if (PreferredFocusWidget && IsInViewport())
	{
		PreferredFocusWidget->SetKeyboardFocus();
	}
}
