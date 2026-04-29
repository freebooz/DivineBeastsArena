// Copyright Epic Games, Inc. All Rights Reserved.

#include "Client/UI/Frontend/HeroSelect/DBAHeroSelectWidgetBase.h"
#include "Client/UI/Frontend/HeroSelect/DBAHeroInfoPanelWidgetBase.h"
#include "Client/UI/Frontend/HeroSelect/DBAHeroSelectWidgetController.h"

UDBAHeroSelectWidgetBase::UDBAHeroSelectWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CurrentSelectedZodiac(EDBAZodiac::None)
	, bHasSelectedZodiac(false)
{
}

void UDBAHeroSelectWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
}

void UDBAHeroSelectWidgetBase::NativeDestruct()
{
	Super::NativeDestruct();
}

void UDBAHeroSelectWidgetBase::NativeOnActivated()
{
	RefreshZodiacList();
}

void UDBAHeroSelectWidgetBase::NativeOnDeactivated()
{
	CurrentSelectedZodiac = EDBAZodiac::None;
	bHasSelectedZodiac = false;
}

void UDBAHeroSelectWidgetBase::SetWidgetController(UDBAHeroSelectWidgetController* InController)
{
	WidgetController = InController;
}

void UDBAHeroSelectWidgetBase::RefreshZodiacList()
{
	TArray<EDBAZodiac> AvailableZodiacs;
	AvailableZodiacs.Add(EDBAZodiac::Rat);
	AvailableZodiacs.Add(EDBAZodiac::Ox);
	AvailableZodiacs.Add(EDBAZodiac::Tiger);
	AvailableZodiacs.Add(EDBAZodiac::Rabbit);
	AvailableZodiacs.Add(EDBAZodiac::Dragon);
	AvailableZodiacs.Add(EDBAZodiac::Snake);
	AvailableZodiacs.Add(EDBAZodiac::Horse);
	AvailableZodiacs.Add(EDBAZodiac::Goat);
	AvailableZodiacs.Add(EDBAZodiac::Monkey);
	AvailableZodiacs.Add(EDBAZodiac::Rooster);
	AvailableZodiacs.Add(EDBAZodiac::Dog);
	AvailableZodiacs.Add(EDBAZodiac::Pig);

	BP_OnRefreshZodiacList(AvailableZodiacs);
}

void UDBAHeroSelectWidgetBase::SelectZodiac(EDBAZodiac Zodiac)
{
	if (Zodiac == EDBAZodiac::None)
	{
		return;
	}

	CurrentSelectedZodiac = Zodiac;
	bHasSelectedZodiac = true;

	BP_OnZodiacSelected(Zodiac);
	BP_OnConfirmButtonStateChanged(true);

	if (HeroInfoPanel)
	{
		HeroInfoPanel->SetZodiac(Zodiac);
	}
}

void UDBAHeroSelectWidgetBase::ConfirmZodiacSelection()
{
	if (!bHasSelectedZodiac || !WidgetController)
	{
		return;
	}

	WidgetController->ConfirmZodiacSelection(CurrentSelectedZodiac);
}

void UDBAHeroSelectWidgetBase::OnBackButtonClicked()
{
	if (WidgetController)
	{
		WidgetController->RequestBack();
	}
}
