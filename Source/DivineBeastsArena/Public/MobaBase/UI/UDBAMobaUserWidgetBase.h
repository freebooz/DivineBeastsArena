// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAUserWidgetBase.h"
#include "UDBAMobaUserWidgetBase.generated.h"

/**
 * MOBA 游戏 UserWidget 基类
 *
 * 提供 MOBA 游戏通用的 UI 组件：
 * - 统一的激活/停用逻辑
 * - HUD 层级管理
 * - 动画和过渡效果基类
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAMobaUserWidgetBase : public UDBAUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBAMobaUserWidgetBase(const FObjectInitializer& ObjectInitializer);

	/**
	 * 获取 HUD 层级
	 */
	UFUNCTION(BlueprintCallable, Category = "MobaBase|UI")
	int32 GetHUDZOrder() const { return ZOrder; }

protected:
	/** HUD 层级（数值越大越靠前） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MobaBase|UI")
	int32 ZOrder = 0;
};
