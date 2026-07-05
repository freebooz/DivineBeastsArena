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

	if (UWorld* World = GetWorld(); World && World->GetNetMode() == NM_DedicatedServer)
	{
		SetComponentTickEnabled(false);
		return;
	}

	CreateOverheadWidget();
}

void UDBAOverheadWidgetComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (OverheadWidget)
	{
		OverheadWidget->RemoveFromParent();
		OverheadWidget = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void UDBAOverheadWidgetComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateWidgetPosition();
}

void UDBAOverheadWidgetComponent::CreateOverheadWidget()
{
	UWorld* World = GetWorld();
	if (!OverheadWidgetClass || !World || World->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	OverheadWidget = CreateWidget<UUserWidget>(World, OverheadWidgetClass);
	if (OverheadWidget)
	{
		OverheadWidget->AddToViewport();
		SetHealthBarPercent(CachedHealthPercent);
		SetCharacterName(CachedCharacterName);
		SetOverheadVisible(bCachedOverheadVisible);
		ApplyWidgetConfig();
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
	CachedCharacterName = Name;

	if (!OverheadWidget)
	{
		return;
	}

	if (UTextBlock* NameText = Cast<UTextBlock>(OverheadWidget->GetWidgetFromName(TEXT("NameText"))))
	{
		NameText->SetText(Name);
	}
}

void UDBAOverheadWidgetComponent::SetOverheadVisible(bool bShouldBeVisible)
{
	bCachedOverheadVisible = bShouldBeVisible;

	if (OverheadWidget)
	{
		OverheadWidget->SetVisibility(bShouldBeVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	}
}

void UDBAOverheadWidgetComponent::ApplyWidgetConfig()
{
	if (!OverheadWidget)
	{
		return;
	}

	if (UWidget* HealthBarWidget = OverheadWidget->GetWidgetFromName(TEXT("HealthBar")))
	{
		HealthBarWidget->SetVisibility(bShowHealthBar ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (UWidget* NameTextWidget = OverheadWidget->GetWidgetFromName(TEXT("NameText")))
	{
		NameTextWidget->SetVisibility(bShowName ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
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
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (APlayerController* PC = World->GetFirstPlayerController())
	{
		FVector2D ScreenPosition;
		if (PC->ProjectWorldLocationToScreen(WorldPosition, ScreenPosition))
		{
			SetOverheadVisible(bCachedOverheadVisible);
			OverheadWidget->SetPositionInViewport(ScreenPosition, false);
		}
		else
		{
			OverheadWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}
