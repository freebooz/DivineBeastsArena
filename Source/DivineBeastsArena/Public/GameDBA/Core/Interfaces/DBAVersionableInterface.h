// Copyright FreeboozStudio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DBAVersionableInterface.generated.h"

/**
 * 可版本化对象接口
 * 用于 SaveGame、Profile、配置文件的版本管理和迁移
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UDBAVersionableInterface : public UInterface
{
	GENERATED_BODY()
};

class DIVINEBEASTSARENA_API IDBAVersionableInterface
{
	GENERATED_BODY()

public:
	/**
	 * 获取当前版本号
	 * @return 版本号
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Version")
	int32 GetVersion() const;
	virtual int32 GetVersion_Implementation() const { return 1; }

	/**
	 * 从旧版本迁移数据
	 * @param OldVersion 旧版本号
	 * @return 是否迁移成功
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Version")
	bool MigrateFromVersion(int32 OldVersion);
	virtual bool MigrateFromVersion_Implementation(int32 OldVersion) { return true; }
};
