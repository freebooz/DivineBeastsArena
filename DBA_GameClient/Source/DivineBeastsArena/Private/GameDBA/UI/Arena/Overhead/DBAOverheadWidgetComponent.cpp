// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 私有实现文件，承载运行时逻辑、网络同步、GAS、UI 或表现层实现。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


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
			SetHealthBarPercent(CachedHealthPercent);
		}
	}
}

void UDBAOverheadWidgetComponent::SetHealthBarPercent(float Percent)
{
	CachedHealthPercent = FMath::Clamp(Percent, 0.0f, 1.0f);

	if (OverheadWidget)
	{
		if (UProgressBar* HealthBar = Cast<UProgressBar>(OverheadWidget->GetWidgetFromName(TEXT("HealthBar"))))
		{
			HealthBar->SetPercent(CachedHealthPercent);
			HealthBar->SetFillColorAndOpacity(HealthBarColor);
		}
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

