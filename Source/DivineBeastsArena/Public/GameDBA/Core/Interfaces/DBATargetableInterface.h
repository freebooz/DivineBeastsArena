// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DBATargetableInterface.generated.h"

/**
 * 基础目标代理接口
 * 用于标识 Actor 是否可被选中、锁定、攻击
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UDBATargetableInterface : public UInterface
{
	GENERATED_BODY()
};

class DIVINEBEASTSARENA_API IDBATargetableInterface
{
	GENERATED_BODY()

public:
	/**
	 * 是否可被选中
	 * @return 是否可被选中
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Targeting")
	bool IsTargetable() const;
	virtual bool IsTargetable_Implementation() const { return true; }

	/**
	 * 获取目标优先级（用于自动选择目标）
	 * @return 优先级，数值越大优先级越高
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Targeting")
	float GetTargetPriority() const;
	virtual float GetTargetPriority_Implementation() const { return 1.0f; }

	/**
	 * 获取目标显示名称
	 * @return 显示名称
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Targeting")
	FText GetTargetDisplayName() const;
	virtual FText GetTargetDisplayName_Implementation() const { return FText::FromString(TEXT("目标")); }
};
