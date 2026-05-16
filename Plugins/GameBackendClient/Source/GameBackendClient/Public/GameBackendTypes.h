// Copyright Freebooz Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameBackendTypes.generated.h"

GAMEBACKENDCLIENT_API DECLARE_LOG_CATEGORY_EXTERN(LogGameBackendClient, Log, All);

DECLARE_DYNAMIC_DELEGATE_ThreeParams(FGameBackendResponseDelegate, bool, bSuccess, const FString&, ErrorMessage, const FString&, DataJson);
DECLARE_DYNAMIC_DELEGATE_FiveParams(FGameBackendAuthResponseDelegate, bool, bSuccess, const FString&, ErrorMessage, const FString&, AccessToken, const FString&, RefreshToken, const FString&, PlayerId);
DECLARE_DYNAMIC_DELEGATE_FourParams(FGameBackendBanResponseDelegate, bool, bSuccess, const FString&, ErrorMessage, const FString&, BanReason, const FString&, UnbanTimeUtc);

USTRUCT(BlueprintType)
struct GAMEBACKENDCLIENT_API FGameBackendApiResponse
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameBackend")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameBackend")
	FString Code;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameBackend")
	FString Message;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameBackend")
	FString DataJson;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameBackend")
	int32 HttpStatus = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameBackend")
	FString TraceId;
};

USTRUCT(BlueprintType)
struct GAMEBACKENDCLIENT_API FGameBackendAuthTokens
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameBackend")
	FString AccessToken;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameBackend")
	FString RefreshToken;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameBackend")
	FString PlayerId;
};

USTRUCT(BlueprintType)
struct GAMEBACKENDCLIENT_API FGameBackendGuestLoginRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameBackend")
	FString DeviceId;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameBackend")
	FString DeviceName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameBackend")
	FString Platform;
};

USTRUCT(BlueprintType)
struct GAMEBACKENDCLIENT_API FGameBackendVersionCheckRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameBackend")
	FString ClientVersion;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameBackend")
	FString BuildNumber;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameBackend")
	FString Channel;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameBackend")
	FString Platform;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameBackend")
	FString Region;
};

USTRUCT(BlueprintType)
struct GAMEBACKENDCLIENT_API FGameBackendRoomCreateRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameBackend")
	FString Mode = TEXT("default");

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameBackend")
	FString Region = TEXT("local");

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameBackend")
	int32 MaxPlayers = 10;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameBackend")
	bool bPrivate = false;
};

USTRUCT(BlueprintType)
struct GAMEBACKENDCLIENT_API FGameBackendSetReadyRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameBackend")
	bool bReady = false;
};

USTRUCT(BlueprintType)
struct GAMEBACKENDCLIENT_API FGameBackendMatchTicketCreateRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameBackend")
	FString Mode;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameBackend")
	FString Region;
};

USTRUCT(BlueprintType)
struct GAMEBACKENDCLIENT_API FGameBackendSessionConnection
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameBackend")
	FString Ip;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameBackend")
	int32 Port = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameBackend")
	FString SessionId;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameBackend")
	FString PlayerSessionToken;
};

USTRUCT(BlueprintType)
struct GAMEBACKENDCLIENT_API FGameBackendTelemetryEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameBackend")
	FString EventName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameBackend")
	TMap<FString, FString> Properties;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameBackend")
	FString TimestampUtc;
};

USTRUCT(BlueprintType)
struct GAMEBACKENDCLIENT_API FGameBackendSupportTicketRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameBackend")
	FString Subject;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameBackend")
	FString Category;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameBackend")
	FString Content;
};

USTRUCT(BlueprintType)
struct GAMEBACKENDCLIENT_API FGameBackendReportRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameBackend")
	FString TargetPlayerId;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameBackend")
	FString ReasonCode;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameBackend")
	FString Description;
};
