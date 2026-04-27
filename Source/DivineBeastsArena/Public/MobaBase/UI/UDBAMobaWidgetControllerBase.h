// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Common/UI/DBAWidgetController.h"
#include "UDBAMobaWidgetControllerBase.generated.h"

/**
 * MOBA 游戏 Widget 控制器基类
 *
 * 提供 MOBA 游戏通用的 Widget 控制逻辑：
 * - 管理 Widget 的显示/隐藏状态
 * - 提供 MOBA 特定的数据更新接口
 * - 处理 HUD 布局和优先级
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAMobaWidgetControllerBase : public UDBAWidgetController
{
	GENERATED_BODY()

public:
	UDBAMobaWidgetControllerBase(const FObjectInitializer& ObjectInitializer);

	/**
	 * 获取 HUD 优先级
	 * 数值越高优先级越高
	 */
	UFUNCTION(BlueprintCallable, Category = "MobaBase|UI")
	int32 GetHUDPriority() const { return HUDPriority; }

protected:
	/** HUD 优先级（数值越高优先级越高） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MobaBase|UI")
	int32 HUDPriority = 0;
};
