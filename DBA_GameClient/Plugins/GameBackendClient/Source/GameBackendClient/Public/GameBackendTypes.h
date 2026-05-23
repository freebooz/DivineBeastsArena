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
#include "GameBackendTypes.generated.h"

GAMEBACKENDCLIENT_API DECLARE_LOG_CATEGORY_EXTERN(LogDBA_GameBackendClient, Log, All);

DECLARE_DYNAMIC_DELEGATE_ThreeParams(FDBA_GameBackendResponseDelegate, bool, bSuccess, const FString&, ErrorMessage, const FString&, DataJson);
DECLARE_DYNAMIC_DELEGATE_FiveParams(FDBA_GameBackendAuthResponseDelegate, bool, bSuccess, const FString&, ErrorMessage, const FString&, AccessToken, const FString&, RefreshToken, const FString&, PlayerId);
DECLARE_DYNAMIC_DELEGATE_FourParams(FDBA_GameBackendBanResponseDelegate, bool, bSuccess, const FString&, ErrorMessage, const FString&, BanReason, const FString&, UnbanTimeUtc);

USTRUCT(BlueprintType)
struct GAMEBACKENDCLIENT_API FDBA_GameBackendApiResponse
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DBA_GameBackend")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DBA_GameBackend")
	FString Code;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DBA_GameBackend")
	FString Message;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DBA_GameBackend")
	FString DataJson;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DBA_GameBackend")
	int32 HttpStatus = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DBA_GameBackend")
	FString TraceId;
};

USTRUCT(BlueprintType)
struct GAMEBACKENDCLIENT_API FDBA_GameBackendAuthTokens
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DBA_GameBackend")
	FString AccessToken;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DBA_GameBackend")
	FString RefreshToken;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DBA_GameBackend")
	FString PlayerId;
};

USTRUCT(BlueprintType)
struct GAMEBACKENDCLIENT_API FDBA_GameBackendGuestLoginRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DBA_GameBackend")
	FString DeviceId;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DBA_GameBackend")
	FString DeviceName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DBA_GameBackend")
	FString Platform;
};

USTRUCT(BlueprintType)
struct GAMEBACKENDCLIENT_API FDBA_GameBackendVersionCheckRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DBA_GameBackend")
	FString ClientVersion;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DBA_GameBackend")
	FString BuildNumber;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DBA_GameBackend")
	FString Channel;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DBA_GameBackend")
	FString Platform;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DBA_GameBackend")
	FString Region;
};

USTRUCT(BlueprintType)
struct GAMEBACKENDCLIENT_API FDBA_GameBackendRoomCreateRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DBA_GameBackend")
	FString Mode = TEXT("default");

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DBA_GameBackend")
	FString Region = TEXT("local");

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DBA_GameBackend")
	int32 MaxPlayers = 10;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DBA_GameBackend")
	bool bPrivate = false;
};

USTRUCT(BlueprintType)
struct GAMEBACKENDCLIENT_API FDBA_GameBackendSetReadyRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DBA_GameBackend")
	bool bReady = false;
};

USTRUCT(BlueprintType)
struct GAMEBACKENDCLIENT_API FDBA_GameBackendMatchTicketCreateRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DBA_GameBackend")
	FString Mode;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DBA_GameBackend")
	FString Region;
};

USTRUCT(BlueprintType)
struct GAMEBACKENDCLIENT_API FDBA_GameBackendSessionConnection
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DBA_GameBackend")
	FString Ip;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DBA_GameBackend")
	int32 Port = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DBA_GameBackend")
	FString SessionId;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DBA_GameBackend")
	FString PlayerSessionToken;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DBA_GameBackend")
	FString PlayerId;
};

USTRUCT(BlueprintType)
struct GAMEBACKENDCLIENT_API FDBA_GameBackendTelemetryEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DBA_GameBackend")
	FString EventName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DBA_GameBackend")
	TMap<FString, FString> Properties;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DBA_GameBackend")
	FString TimestampUtc;
};

USTRUCT(BlueprintType)
struct GAMEBACKENDCLIENT_API FDBA_GameBackendSupportTicketRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DBA_GameBackend")
	FString Subject;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DBA_GameBackend")
	FString Category;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DBA_GameBackend")
	FString Content;
};

USTRUCT(BlueprintType)
struct GAMEBACKENDCLIENT_API FDBA_GameBackendReportRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DBA_GameBackend")
	FString TargetPlayerId;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DBA_GameBackend")
	FString ReasonCode;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "DBA_GameBackend")
	FString Description;
};
