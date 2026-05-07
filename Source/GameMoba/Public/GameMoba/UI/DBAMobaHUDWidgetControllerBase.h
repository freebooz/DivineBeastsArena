// Copyright Freebooz Games, Inc. All Rights Reserved.
// GameMoba - 通用MOBA HUD WidgetController基类

#pragma once

#include "CoreMinimal.h"
#include "UObject/UObject.h"
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
	UDBAMobaHUDWidgetControllerBase(const FObjectInitializer& ObjectInitializer);

	/** 初始化WidgetController */
	UFUNCTION(BlueprintCallable, Category = "DBA|HUD|WidgetController")
	virtual void InitializeController(class APlayerController* InPlayerController);

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