// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/

// GameMoba - 通用MOBA HUD WidgetController基类

#pragma once

#include "CoreMinimal.h"
#include "DBAMobaHUDWidgetControllerBase.generated.h"

/**
 * UDBAMobaHUDWidgetControllerBase
 * MOBA游戏通用HUD WidgetController基类
 * 用于解耦HUD数据与显示
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class GAMEMOBA_API UDBAMobaHUDWidgetControllerBase : public UObject
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerHPChanged, float, CurrentHP, float, MaxHP);

	UDBAMobaHUDWidgetControllerBase(const FObjectInitializer& ObjectInitializer);

	/** 初始化WidgetController */
	UFUNCTION(BlueprintCallable, Category = "DBA|HUD|WidgetController")
	virtual void InitializeController(class APlayerController* InPlayerController);

	UFUNCTION(BlueprintCallable, Category = "DBA|HUD|WidgetController")
	virtual void UpdatePlayerHP(float CurrentHP, float MaxHP);

	UPROPERTY(BlueprintAssignable, Category = "DBA|HUD|WidgetController")
	FOnPlayerHPChanged OnPlayerHPChanged;

	/** 获取PlayerController */
	UFUNCTION(BlueprintCallable, Category = "DBA|HUD|WidgetController")
	class APlayerController* GetPlayerController() const { return PlayerController.Get(); }

protected:
	/** 所属PlayerController */
	UPROPERTY()
	TWeakObjectPtr<class APlayerController> PlayerController;

	/** 是否已初始化 */
	UPROPERTY(BlueprintReadOnly, Category = "DBA|HUD|WidgetController")
	bool bIsInitialized = false;
};
