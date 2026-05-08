// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameMoba/UI/UDBAMobaUserWidgetBase.h"
#include "UDBALoginFlowWidgetBase.generated.h"

/**
 * UDBALoginFlowWidgetBase
 * 登录流程Widget基类
 * 提供登录界面通用功能
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBALoginFlowWidgetBase : public UDBAMobaUserWidgetBase
{
	GENERATED_BODY()

public:
	UDBALoginFlowWidgetBase(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

public:
	/** 显示错误信息 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Login")
	virtual void ShowError(const FString& ErrorMessage);

	/** 清除错误信息 */
	UFUNCTION(BlueprintCallable, Category = "DBA|Login")
	virtual void ClearError();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|Login", meta = (DisplayName = "On Show Error"))
	void BP_OnShowError(const FString& ErrorMessage);

	UFUNCTION(BlueprintImplementableEvent, Category = "DBA|Login", meta = (DisplayName = "On Clear Error"))
	void BP_OnClearError();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "DBA|Login")
	FString LastErrorMessage;
};
