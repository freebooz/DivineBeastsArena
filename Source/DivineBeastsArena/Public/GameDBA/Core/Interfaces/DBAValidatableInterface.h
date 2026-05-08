// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DBAValidatableInterface.generated.h"

/**
 * 可校验对象接口
 * 用于 DataTable、DataAsset、配置对象的数据完整性校验
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UDBAValidatableInterface : public UInterface
{
	GENERATED_BODY()
};

class DIVINEBEASTSARENA_API IDBAValidatableInterface
{
	GENERATED_BODY()

public:
	/**
	 * 校验数据完整性
	 * @param OutErrors 输出错误列表
	 * @return 是否校验通过
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Validation")
	bool ValidateData(TArray<FString>& OutErrors) const;
	virtual bool ValidateData_Implementation(TArray<FString>& OutErrors) const { return true; }
};
