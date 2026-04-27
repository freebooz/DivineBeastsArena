// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DBAWidgetBase.generated.h"

/**
 * DBAWidgetBase
 *
 * UI 控件基类
 * 提供通用的 Widget 功能
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAWidgetBase : public UObject
{
	GENERATED_BODY()

public:
	UDBAWidgetBase(const FObjectInitializer& ObjectInitializer);

	/**
	 * 获取 Widget 的初始名字
	 */
	UFUNCTION(BlueprintPure, Category = "DBA|UI")
	FString GetWidgetName() const { return WidgetName; }

protected:
	/** Widget 名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI")
	FString WidgetName;
};
