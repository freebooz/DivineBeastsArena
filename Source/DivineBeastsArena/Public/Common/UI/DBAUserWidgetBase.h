// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DBAUserWidgetBase.generated.h"

/**
 * DBAUserWidgetBase
 *
 * UMG UserWidget 基类
 * 用于所有可在屏幕上显示的 UI 控件
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAUserWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UDBAUserWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	/**
	 * Widget 被激活时调用（显示时）
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI")
	virtual void Activate();

	/**
	 * Widget 被取消激活时调用（隐藏时）
	 */
	UFUNCTION(BlueprintCallable, Category = "DBA|UI")
	virtual void Deactivate();

	/**
	 * 检查 Widget 是否处于激活状态
	 */
	UFUNCTION(BlueprintPure, Category = "DBA|UI")
	bool IsActive() const { return bIsActive; }

protected:
	/** Widget 是否处于激活状态 */
	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI")
	bool bIsActive = false;

	/** Widget 的优先级（数值越高优先级越高） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA|UI")
	int32 WidgetPriority = 0;
};
