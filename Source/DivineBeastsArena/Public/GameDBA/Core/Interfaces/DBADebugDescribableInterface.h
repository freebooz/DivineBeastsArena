// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DBADebugDescribableInterface.generated.h"

/**
 * 可调试描述对象接口
 * 用于生成调试信息、日志输出、开发者工具显示
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UDBADebugDescribableInterface : public UInterface
{
	GENERATED_BODY()
};

class DIVINEBEASTSARENA_API IDBADebugDescribableInterface
{
	GENERATED_BODY()

public:
	/**
	 * 获取调试描述字符串
	 * @return 调试描述
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Debug")
	FString GetDebugDescription() const;
	virtual FString GetDebugDescription_Implementation() const { return TEXT("No Description"); }
};
