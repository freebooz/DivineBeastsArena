// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DBATelemetryInterface.generated.h"

/**
 * 可遥测对象接口
 * 用于标识对象支持可选的外部 Monitoring 遥测上报
 * 遥测功能不影响游戏核心逻辑
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UDBATelemetryInterface : public UInterface
{
	GENERATED_BODY()
};

class DIVINEBEASTSARENA_API IDBATelemetryInterface
{
	GENERATED_BODY()

public:
	/**
	 * 是否启用遥测
	 * @return 是否启用
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Telemetry")
	bool IsTelemetryEnabled() const;
	virtual bool IsTelemetryEnabled_Implementation() const { return false; }

	/**
	 * 收集遥测数据
	 * @return 遥测数据（JSON 格式）
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Telemetry")
	FString CollectTelemetryData() const;
	virtual FString CollectTelemetryData_Implementation() const { return TEXT("{}"); }
};
