// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MobaBase/Input/DBAInputMappingContextTypes.h"
#include "DBAInputPlatformPolicy.generated.h"

/**
 * 输入平台策略接口
 * 用于判断当前运行平台并选择对应的输入映射上下文
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class DIVINEBEASTSARENA_API UDBAInputPlatformPolicy : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * 获取当前平台类型
	 * @return 当前运行的平台类型
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Input Platform")
	EDBAInputPlatform GetCurrentPlatform() const;
	virtual EDBAInputPlatform GetCurrentPlatform_Implementation() const;

	/**
	 * 判断是否为触屏平台
	 * @return 是否为触屏平台（Android）
	 */
	UFUNCTION(BlueprintCallable, Category = "Input Platform")
	bool IsTouchPlatform() const;
};
