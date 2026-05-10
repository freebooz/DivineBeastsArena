// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/UI/DBAUserWidgetBase.h"
#include "DBASpectatorMinimapWidgetBase.generated.h"

class UImage;
class UCanvasPanel;
class UButton;

/**
 * UDBASpectatorMinimapWidgetBase
 * 观战小地图Widget
 * 显示所有玩家位置，点击可切换视角
 */
UCLASS(Abstract, Blueprintable)
class DIVINEBEASTSARENA_API UDBASpectatorMinimapWidgetBase : public UDBAUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBASpectatorMinimapWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;

public:
	/** 更新小地图显示 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator|Minimap")
	void UpdateMinimap(const TArray<FDBAObserverViewTarget>& AllTargets, int32 CurrentTargetIndex);

	/** 设置缩放比例 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator|Minimap")
	void SetZoom(float NewZoom);

	/** 获取缩放比例 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator|Minimap")
	float GetZoom() const { return MapZoom; }

protected:
	/** 点击玩家头像回调 */
	UFUNCTION()
	void OnPlayerButtonClicked(int32 PlayerIndex);

	/** 世界坐标转小地图坐标 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator|Minimap")
	FVector2D WorldToMinimap(const FVector& WorldPos) const;

	/** 设置地图原点 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator|Minimap")
	void SetMapOrigin(const FVector& Origin);

	/** 获取地图原点 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Spectator|Minimap")
	FVector GetMapOrigin() const { return MapOrigin; }

protected:
	/** 玩家头像按钮数组 */
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<UButton>> PlayerButtons;

	/** 玩家位置点数组 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TArray<UImage*> PlayerDots;

	/** 队伍1颜色 */
	UPROPERTY(EditDefaultsOnly, Category = "DBA|Spectator|Minimap")
	FLinearColor Team1Color;

	/** 队伍2颜色 */
	UPROPERTY(EditDefaultsOnly, Category = "DBA|Spectator|Minimap")
	FLinearColor Team2Color;

	/** 当前视角索引 */
	UPROPERTY(Transient)
	int32 CurrentTargetIndex;

	/** 地图缩放 */
	UPROPERTY(EditDefaultsOnly, Category = "DBA|Spectator|Minimap")
	float MapZoom;

	/** 小地图尺寸 */
	UPROPERTY(EditDefaultsOnly, Category = "DBA|Spectator|Minimap")
	FVector2D MinimapSize;

	/** 地图原点 (世界坐标) */
	UPROPERTY(EditDefaultsOnly, Category = "DBA|Spectator|Minimap")
	FVector MapOrigin;

	/** 世界坐标到小地图的比例 (小地图像素/世界单位) */
	UPROPERTY(EditDefaultsOnly, Category = "DBA|Spectator|Minimap")
	float MapScale;
};
