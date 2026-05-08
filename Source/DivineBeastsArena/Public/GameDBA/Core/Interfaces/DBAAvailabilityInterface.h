// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DBAAvailabilityInterface.generated.h"

/**
 * 可用性查询接口
 * 用于查询系统、功能、服务的可用状态
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UDBAAvailabilityInterface : public UInterface
{
	GENERATED_BODY()
};

class DIVINEBEASTSARENA_API IDBAAvailabilityInterface
{
	GENERATED_BODY()

public:
	/**
	 * 查询是否可用
	 * @return 是否可用
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Availability")
	bool IsAvailable() const;
	virtual bool IsAvailable_Implementation() const { return true; }

	/**
	 * 获取不可用原因
	 * @return 不可用原因描述
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Availability")
	FString GetUnavailableReason() const;
	virtual FString GetUnavailableReason_Implementation() const { return FString(); }
};
