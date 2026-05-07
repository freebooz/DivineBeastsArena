// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Spectator/UI/DBASpectatorMinimapWidgetBase.h"

#include "Components/Image.h"
#include "GameDBA/Character/DBAZodiacCharacterBase.h"

UDBASpectatorMinimapWidgetBase::UDBASpectatorMinimapWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, Team1Color(FLinearColor::Blue)
	, Team2Color(FLinearColor::Red)
	, CurrentTargetIndex(INDEX_NONE)
	, MapZoom(1.0f)
	, MinimapSize(200.0f, 200.0f)
	, MapOrigin(FVector::ZeroVector)
	, MapScale(0.1f)
{
}

void UDBASpectatorMinimapWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
}

void UDBASpectatorMinimapWidgetBase::UpdateMinimap(const TArray<FDBAObserverViewTarget>& AllTargets, int32 InCurrentTargetIndex)
{
	CurrentTargetIndex = InCurrentTargetIndex;
	for (int32 Index = 0; Index < AllTargets.Num() && Index < PlayerDots.Num(); ++Index)
	{
		UImage* Dot = PlayerDots[Index];
		if (!Dot)
		{
			continue;
		}

		const FDBAObserverViewTarget& Target = AllTargets[Index];
		FLinearColor DotColor = Target.TeamID == 0 ? Team1Color : Team2Color;
		if (Index == CurrentTargetIndex)
		{
			DotColor.R += 0.3f;
			DotColor.G += 0.3f;
			DotColor.B += 0.3f;
		}

		Dot->SetColorAndOpacity(DotColor);
		if (ADBAZodiacCharacterBase* Character = Target.TargetCharacter.Get())
		{
			Dot->SetRenderTranslation(WorldToMinimap(Character->GetActorLocation()));
		}
		Dot->SetVisibility(ESlateVisibility::Visible);
	}
}

FVector2D UDBASpectatorMinimapWidgetBase::WorldToMinimap(const FVector& WorldPos) const
{
	const FVector RelativePos = WorldPos - MapOrigin;
	FVector2D MapPos(RelativePos.X * MapScale * MapZoom + MinimapSize.X * 0.5f, RelativePos.Y * MapScale * MapZoom + MinimapSize.Y * 0.5f);
	MapPos.X = FMath::Clamp(MapPos.X, 0.0f, MinimapSize.X);
	MapPos.Y = FMath::Clamp(MapPos.Y, 0.0f, MinimapSize.Y);
	return MapPos;
}

void UDBASpectatorMinimapWidgetBase::SetMapOrigin(const FVector& Origin)
{
	MapOrigin = Origin;
}

void UDBASpectatorMinimapWidgetBase::SetZoom(float NewZoom)
{
	MapZoom = FMath::Clamp(NewZoom, 0.5f, 3.0f);
}

void UDBASpectatorMinimapWidgetBase::OnPlayerButtonClicked(int32 PlayerIndex)
{
}