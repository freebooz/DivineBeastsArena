// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


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
