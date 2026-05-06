// Copyright FreeboozStudio. All Rights Reserved.

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
	bool bAllowMockFallback = true;

	FString GetBaseUrl() const
	{
		return FString::Printf(TEXT("http://%s:%d"), *ServerHost, ServerPort);
	}
};
