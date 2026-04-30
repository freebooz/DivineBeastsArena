// Copyright Epic Games, Inc. All Rights Reserved.

#include "Client/UI/Lobby/ElementSelect/DBAElementSelectWidgetBase.h"
#include "Client/UI/Lobby/ElementSelect/DBAElementInfoPanelWidgetBase.h"
#include "Client/UI/Lobby/ElementSelect/DBAFixedSkillGroupPreviewWidgetBase.h"
#include "Client/UI/Lobby/ElementSelect/DBAElementSelectWidgetController.h"

UDBAElementSelectWidgetBase::UDBAElementSelectWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, SelectedZodiac(EDBAZodiac::None)
	, CurrentSelectedElement(EDBAElement::None)
	, bHasSelectedElement(false)
{
}

void UDBAElementSelectWidgetBase::NativeConstruct()
{
	// UDBAWidgetBase does not have NativeConstruct/NativeDestruct/NativeOnActivated/NativeOnDeactivated
	// The base class is UObject, not UUserWidget
}

void UDBAElementSelectWidgetBase::NativeDestruct()
{
}

void UDBAElementSelectWidgetBase::NativeOnActivated()
{
	RefreshElementList();
}

void UDBAElementSelectWidgetBase::NativeOnDeactivated()
{
	CurrentSelectedElement = EDBAElement::None;
	bHasSelectedElement = false;
}

void UDBAElementSelectWidgetBase::SetWidgetController(UDBAElementSelectWidgetController* InController)
{
	WidgetController = InController;
}

void UDBAElementSelectWidgetBase::SetSelectedZodiac(EDBAZodiac Zodiac)
{
	SelectedZodiac = Zodiac;
}

void UDBAElementSelectWidgetBase::RefreshElementList()
{
	TArray<EDBAElement> AvailableElements;
	AvailableElements.Add(EDBAElement::Gold);
	AvailableElements.Add(EDBAElement::Wood);
	AvailableElements.Add(EDBAElement::Water);
	AvailableElements.Add(EDBAElement::Fire);
	AvailableElements.Add(EDBAElement::Earth);

	BP_OnRefreshElementList(AvailableElements);
}

void UDBAElementSelectWidgetBase::SelectElement(EDBAElement Element)
{
	if (Element == EDBAElement::None)
	{
		return;
	}

	CurrentSelectedElement = Element;
	bHasSelectedElement = true;

	BP_OnElementSelected(Element);
	BP_OnConfirmButtonStateChanged(true);

	if (ElementInfoPanel)
	{
		ElementInfoPanel->SetElement(Element);
	}

	if (SkillGroupPreview)
	{
		SkillGroupPreview->SetZodiacAndElement(SelectedZodiac, Element);
	}
}

void UDBAElementSelectWidgetBase::ConfirmElementSelection()
{
	if (!bHasSelectedElement || !WidgetController)
	{
		return;
	}

	WidgetController->ConfirmElementSelection(CurrentSelectedElement);
}

void UDBAElementSelectWidgetBase::OnBackButtonClicked()
{
	if (WidgetController)
	{
		WidgetController->RequestBack();
	}
}
