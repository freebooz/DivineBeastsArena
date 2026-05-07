// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/UI/Arena/Overhead/DBAOverheadWidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

UDBAOverheadWidgetComponent::UDBAOverheadWidgetComponent()
	: Super()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bTickEvenWhenPaused = false;
}

void UDBAOverheadWidgetComponent::BeginPlay()
{
	Super::BeginPlay();
	CreateOverheadWidget();
}

void UDBAOverheadWidgetComponent::CreateOverheadWidget()
{
	if (!OverheadWidgetClass)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		OverheadWidget = CreateWidget<UUserWidget>(World, OverheadWidgetClass);
		if (OverheadWidget)
		{
			OverheadWidget->AddToViewport();
			OverheadWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
	}
}

void UDBAOverheadWidgetComponent::SetHealthBarPercent(float Percent)
{
	CachedHealthPercent = FMath::Clamp(Percent, 0.0f, 1.0f);

	if (UProgressBar* HealthBar = Cast<UProgressBar>(OverheadWidget->GetWidgetFromName(TEXT("HealthBar"))))
	{
		HealthBar->SetPercent(CachedHealthPercent);
	}
}

void UDBAOverheadWidgetComponent::SetCharacterName(const FText& Name)
{
	if (UTextBlock* NameText = Cast<UTextBlock>(OverheadWidget->GetWidgetFromName(TEXT("NameText"))))
	{
		NameText->SetText(Name);
	}
}

void UDBAOverheadWidgetComponent::SetOverheadVisible(bool bShouldBeVisible)
{
	if (OverheadWidget)
	{
		OverheadWidget->SetVisibility(bShouldBeVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

FVector UDBAOverheadWidgetComponent::GetOwnerBoundingBoxCenter() const
{
	if (AActor* Owner = GetOwner())
	{
		FVector Origin;
		FVector BoxExtent;
		Owner->GetActorBounds(false, Origin, BoxExtent);
		return Origin + FVector(0.0f, 0.0f, BoxExtent.Z + HealthBarHeightOffset);
	}
	return GetComponentLocation();
}

void UDBAOverheadWidgetComponent::UpdateWidgetPosition()
{
	if (!OverheadWidget)
	{
		return;
	}

	FVector WorldPosition = GetOwnerBoundingBoxCenter();

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		FVector2D ScreenPosition;
		if (PC->ProjectWorldLocationToScreen(WorldPosition, ScreenPosition))
		{
			OverheadWidget->SetPositionInViewport(ScreenPosition, false);
		}
	}
}

