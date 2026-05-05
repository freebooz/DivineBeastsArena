// Copyright Freebooz Games, Inc. All Rights Reserved.

#include "GameDBA/Spectator/UI/DBASpectatorMinimapWidgetBase.h"

UDBASpectatorMinimapWidgetBase::UDBASpectatorMinimapWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, Team1Color(FLinearColor::Blue)
	, Team2Color(FLinearColor::Red)
	, CurrentTargetIndex(INDEX_NONE)
	, MapZoom(1.0f)
	, MapOrigin(FVector::ZeroVector)
	, MapScale(0.1f)  // 默认: 1世界单位 = 0.1小地图像素
{
	MinimapSize = FVector2D(200.0f, 200.0f);
}

void UDBASpectatorMinimapWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	// 初始化玩家按钮
	// 实际应该在Blueprint或运行时动态创建
}

void UDBASpectatorMinimapWidgetBase::UpdateMinimap(const TArray<FDBAObserverViewTarget>& AllTargets, int32 InCurrentTargetIndex)
{
	CurrentTargetIndex = InCurrentTargetIndex;

	// 更新玩家位置点
	for (int32 i = 0; i < AllTargets.Num() && i < PlayerDots.Num(); ++i)
	{
		if (UImage* Dot = PlayerDots[i])
		{
			const FDBAObserverViewTarget& Target = AllTargets[i];

			// 根据队伍设置颜色
			FLinearColor DotColor = Target.TeamID == 0 ? Team1Color : Team2Color;

			// 如果是当前观看目标，加亮显示
			if (i == CurrentTargetIndex)
			{
				DotColor.R += 0.3f;
				DotColor.G += 0.3f;
				DotColor.B += 0.3f;
			}

			Dot->SetColorAndOpacity(DotColor);

			// 计算世界坐标到小地图坐标的映射
			if (AActor* Character = Target.TargetCharacter.Get())
			{
				FVector WorldPos = Character->GetActorLocation();
				FVector2D MapPos = WorldToMinimap(WorldPos);
				Dot->SetRenderTranslation(MapPos);
			}

			Dot->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

FVector2D UDBASpectatorMinimapWidgetBase::WorldToMinimap(const FVector& WorldPos) const
{
	// 计算相对位置
	FVector RelativePos = WorldPos - MapOrigin;

	// 应用缩放
	float ScaledX = RelativePos.X * MapScale * MapZoom;
	float ScaledY = RelativePos.Y * MapScale * MapZoom;

	// 居中在小地图内
	FVector2D MapPos(ScaledX + MinimapSize.X * 0.5f, ScaledY + MinimapSize.Y * 0.5f);

	// 限制在小地图范围内
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
	// 触发切换视角
	// 需要通过delegate或直接调用SpectatorComponent
}
