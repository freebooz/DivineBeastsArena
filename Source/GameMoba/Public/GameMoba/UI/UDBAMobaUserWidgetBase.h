// Copyright Freebooz Games, Inc. All Rights Reserved.
// GameMoba - 通用MOBA用户界面基类

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UDBAMobaUserWidgetBase.generated.h"

/**
 * UDBAMobaUserWidgetBase
 * MOBA游戏通用用户界面基类
 * 提供UI通用接口和事件
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class GAMEMOBA_API UDBAMobaUserWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UDBAMobaUserWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	//~ Begin UUserWidget Interface
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	//~ End UUserWidget Interface

protected:
	/** 显示UI时的动画 */
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|Base", meta = (DisplayName = "On Show"))
	void BP_OnShow();

	/** 隐藏UI时的动画 */
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|UI|Base", meta = (DisplayName = "On Hide"))
	void BP_OnHide();

public:
	/** 是否可见 */
	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Base")
	bool bIsVisible = true;

	/** UI层级 */
	UPROPERTY(BlueprintReadOnly, Category = "DBA|UI|Base")
	int32 UILevel = 0;

protected:
	/** 所属PlayerController */
	UPROPERTY()
	TWeakObjectPtr<class APlayerController> OwnerPlayerController;
};