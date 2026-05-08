// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DBAExternalServiceDegradableInterface.generated.h"

/**
 * 可外部服务降级对象接口
 * 用于标识对象支持外部服务（Monitoring / GameOps）降级模式
 * 外部服务不可用时，对象必须能够正常工作
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UDBAExternalServiceDegradableInterface : public UInterface
{
	GENERATED_BODY()
};

class DIVINEBEASTSARENA_API IDBAExternalServiceDegradableInterface
{
	GENERATED_BODY()

public:
	/**
	 * 是否处于降级模式
	 * @return 是否降级
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "ExternalService")
	bool IsDegraded() const;
	virtual bool IsDegraded_Implementation() const { return false; }

	/**
	 * 进入降级模式
	 * @param Reason 降级原因
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "ExternalService")
	void EnterDegradedMode(const FString& Reason);
	virtual void EnterDegradedMode_Implementation(const FString& Reason) {}

	/**
	 * 退出降级模式
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "ExternalService")
	void ExitDegradedMode();
	virtual void ExitDegradedMode_Implementation() {}
};
