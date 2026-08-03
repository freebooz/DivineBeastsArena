// Copyright Freebooz Games, Inc. All Rights Reserved.
/*
中文阅读说明：
- 所属应用：DBA_GameClient Unreal Engine 客户端 / Dedicated Server。
- 文件职责：声明 UE Dedicated Server 调用 DBA_GameBackend Runtime API 的服务。
- 阅读重点：Dedicated Server 启动参数会注入 SessionId、ServerId、RuntimeToken，本服务负责注册、就绪、心跳和玩家进出上报。
- 修改提示：这里只处理 HTTP 协议封装，权威战报和结算数据仍应由对局规则层生成后再提交。
*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameBackendTypes.h"
#include "GameBackendRuntimeService.generated.h"

class UDBA_GameBackendClientSubsystem;
class FDBA_GameBackendHttpClient;
class FJsonObject;
struct FDBA_GameBackendHttpResult;

USTRUCT(BlueprintType)
struct GAMEBACKENDCLIENT_API FDBA_GameBackendRuntimePlayerBuildSummary
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA_GameBackend|Runtime")
	FString Zodiac;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA_GameBackend|Runtime")
	FString PrimaryElement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA_GameBackend|Runtime")
	FString FiveCamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA_GameBackend|Runtime")
	FString FixedSkillGroupId;

	bool HasAnyValue() const
	{
		return !Zodiac.IsEmpty()
			|| !PrimaryElement.IsEmpty()
			|| !FiveCamp.IsEmpty()
			|| !FixedSkillGroupId.IsEmpty();
	}
};

USTRUCT(BlueprintType)
struct GAMEBACKENDCLIENT_API FDBA_GameBackendRuntimePlayerResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA_GameBackend|Runtime")
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA_GameBackend|Runtime")
	FString Team;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA_GameBackend|Runtime")
	FString Result;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA_GameBackend|Runtime")
	int32 Kills = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA_GameBackend|Runtime")
	int32 Deaths = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA_GameBackend|Runtime")
	int32 Assists = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA_GameBackend|Runtime")
	int32 Score = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA_GameBackend|Runtime")
	int64 ExpDelta = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DBA_GameBackend|Runtime")
	TMap<FString, int32> Rewards;
};

UCLASS(BlueprintType)
class GAMEBACKENDCLIENT_API UDBA_GameBackendRuntimeService : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UDBA_GameBackendClientSubsystem* InSubsystem, FDBA_GameBackendHttpClient* InHttpClient);

	UFUNCTION(BlueprintCallable, Category = "DBA_GameBackend|Runtime")
	bool ConfigureFromCommandLine();

	UFUNCTION(BlueprintPure, Category = "DBA_GameBackend|Runtime")
	bool IsConfigured() const;

	UFUNCTION(BlueprintPure, Category = "DBA_GameBackend|Runtime")
	const FString& GetSessionId() const { return SessionId; }

	UFUNCTION(BlueprintPure, Category = "DBA_GameBackend|Runtime")
	const FString& GetServerId() const { return ServerId; }

	UFUNCTION(BlueprintCallable, Category = "DBA_GameBackend|Runtime")
	void RegisterServer(const FDBA_GameBackendResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "DBA_GameBackend|Runtime")
	void MarkReady(const FDBA_GameBackendResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "DBA_GameBackend|Runtime")
	void SendHeartbeat(const FDBA_GameBackendResponseDelegate& Callback);

	/** 在创建 PlayerController 前异步验证并消费一次性 JoinTicket。 */
	void ValidateJoinTicket(
		const FString& PlayerId,
		const FString& CharacterId,
		const FString& JoinTicket,
		const FString& Team,
		int32 SlotIndex,
		const FDBA_GameBackendRuntimePlayerBuildSummary& BuildSummary,
		FDBA_GameBackendNativeResponseCallback Callback);

	UFUNCTION(BlueprintCallable, Category = "DBA_GameBackend|Runtime")
	void NotifyPlayerLeft(const FString& PlayerId, const FDBA_GameBackendResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "DBA_GameBackend|Runtime")
	void NotifyMatchStarted(const FDBA_GameBackendResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "DBA_GameBackend|Runtime")
	void NotifyMatchEnded(const FDBA_GameBackendResponseDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "DBA_GameBackend|Runtime")
	void NotifyMatchResults(const FString& IdempotencyKey, const FString& ResultJson, const TArray<FDBA_GameBackendRuntimePlayerResult>& Players, const FDBA_GameBackendResponseDelegate& Callback);

	static FString BuildMatchResultsPayload(
		const FString& ServerId,
		const FString& SessionId,
		const FString& RuntimeToken,
		const FString& IdempotencyKey,
		const FString& ResultJson,
		const TArray<FDBA_GameBackendRuntimePlayerResult>& Players);

private:
	FString BuildRuntimePayload(const TFunction<void(TSharedRef<FJsonObject>)>& Fill) const;
	void PostRuntime(const FString& Path, const FString& Body, const FDBA_GameBackendResponseDelegate& Callback) const;
	void PostRuntimeNative(const FString& Path, const FString& Body, FDBA_GameBackendNativeResponseCallback Callback) const;
	static void ExecuteResponse(const FDBA_GameBackendResponseDelegate& Callback, const FDBA_GameBackendHttpResult& Result);

private:
	TWeakObjectPtr<UDBA_GameBackendClientSubsystem> Subsystem;
	FDBA_GameBackendHttpClient* HttpClient = nullptr;

	FString SessionId;
	FString ServerId;
	FString BuildId;
	FString RuntimeToken;
};
