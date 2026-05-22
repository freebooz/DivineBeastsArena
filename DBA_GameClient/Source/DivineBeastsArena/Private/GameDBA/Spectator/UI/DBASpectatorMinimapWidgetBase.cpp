// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


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