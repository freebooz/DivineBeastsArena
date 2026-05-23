// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameBackend 后端 API / Worker。
- 文件职责：Unreal C++ 公共头文件，声明可被其他模块或蓝图使用的类型、属性和函数契约。
- 阅读重点：先看公开类型、路由/组件入口和构造函数，再看私有辅助方法，理解数据如何从输入流向状态变更或界面输出。
- 修改提示：保持现有分层边界；新增逻辑优先复用本目录已有服务、DTO、组件和工具函数，避免把配置、IO 与业务规则混在一起。
*/


#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameBackendClientSettings.generated.h"

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Game Backend Client"))
class GAMEBACKENDCLIENT_API UDBA_GameBackendClientSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UDBA_GameBackendClientSettings();

	virtual FName GetCategoryName() const override;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Connection")
	FString BackendBaseUrl = TEXT("http://localhost:8080");

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Client")
	FString ClientVersion = TEXT("0.1.0");

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Client")
	FString BuildNumber = TEXT("100");

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Client")
	FString ConfigVersion = TEXT("bootstrap_v1");

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Client")
	FString Channel = TEXT("dev");

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Client")
	FString Platform = TEXT("Windows");

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Client")
	FString Region = TEXT("local");

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "HTTP", meta = (ClampMin = "1.0"))
	float RequestTimeoutSeconds = 15.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Telemetry", meta = (ClampMin = "1.0"))
	float TelemetryFlushIntervalSeconds = 10.0f;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Telemetry", meta = (ClampMin = "10"))
	int32 TelemetryMaxQueueSize = 1000;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Telemetry")
	bool bEnableTelemetry = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Crash")
	bool bEnableCrashUpload = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "HTTP", meta = (ClampMin = "0"))
	int32 HttpRetryCount = 1;
};
