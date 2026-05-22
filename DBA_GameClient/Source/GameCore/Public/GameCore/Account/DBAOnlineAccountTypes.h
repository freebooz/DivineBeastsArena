// Copyright FreeboozStudio. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#pragma once

#include "CoreMinimal.h"
#include "DBAOnlineAccountTypes.generated.h"

UENUM(BlueprintType)
enum class EDBAOnlineAccountError : uint8
{
	None,
	NetworkUnavailable,
	Timeout,
	EndpointMissing,
	ServiceUnavailable,
	InvalidCredentials,
	AccountUnavailable,
	TokenExpired,
	ValidationFailed,
	MalformedResponse
};

USTRUCT(BlueprintType)
struct GAMECORE_API FDBAOnlineAccountConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Account|Online")
	FString ServerHost = TEXT("127.0.0.1");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Account|Online")
	int32 ServerPort = 8080;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Account|Online")
	float RequestTimeoutSeconds = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Account|Online")
	bool bAllowMockFallback = false;

	FString GetBaseUrl() const
	{
		return FString::Printf(TEXT("http://%s:%d"), *ServerHost, ServerPort);
	}
};
