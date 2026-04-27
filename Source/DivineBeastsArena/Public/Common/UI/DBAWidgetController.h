// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DBAWidgetController.generated.h"

/**
 * DBAWidgetController
 *
 * Widget 控制器基类
 * 负责管理 Widget 的数据和业务逻辑
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAWidgetController : public UObject
{
	GENERATED_BODY()

public:
	UDBAWidgetController(const FObjectInitializer& ObjectInitializer);

	/**
	 * 初始化控制器
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI")
	virtual void InitializeController();

	/**
	 * 重置控制器状态
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI")
	virtual void ResetController();

protected:
	/** 控制器是否已初始化 */
	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI")
	bool bIsInitialized = false;
};
