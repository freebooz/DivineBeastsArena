// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Lobby/DBALobbyMonsterHealthBarWidget.h"

#include "SlateOptMacros.h"
#include "Widgets/Notifications/SProgressBar.h"

TSharedRef<SWidget> UDBALobbyMonsterHealthBarWidget::RebuildWidget()
{
	HealthProgressBar = SNew(SProgressBar)
		.Percent(CachedPercent)
		.FillColorAndOpacity(ResolveFillColor());
	return HealthProgressBar.ToSharedRef();
}

void UDBALobbyMonsterHealthBarWidget::SetHealthPercent(float Percent)
{
	CachedPercent = FMath::Clamp(Percent, 0.0f, 1.0f);
	if (HealthProgressBar.IsValid())
	{
		HealthProgressBar->SetPercent(CachedPercent);
		HealthProgressBar->SetFillColorAndOpacity(ResolveFillColor());
	}
}

void UDBALobbyMonsterHealthBarWidget::SetSelected(bool bSelected)
{
	bCachedSelected = bSelected;
	if (HealthProgressBar.IsValid())
	{
		HealthProgressBar->SetFillColorAndOpacity(ResolveFillColor());
	}
}

FLinearColor UDBALobbyMonsterHealthBarWidget::ResolveFillColor() const
{
	return bCachedSelected
		? FLinearColor(1.0f, 0.0f, 0.0f, 1.0f)
		: FLinearColor(0.80f, 0.05f, 0.03f, 1.0f);
}
